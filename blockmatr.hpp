#include <iostream>

int cash = 610;
template<class T> class blockmatrix {
public:
    blockmatrix(size_t lines, size_t columns): lines(lines), columns(columns) 
    { // subject to change
        bigbody = new T[lines * columns];
        body = new T*[lines];
        for (size_t i = 0; i < lines; i++) {
            body[i] = bigbody + i*columns;
        }
    }

    ~blockmatrix() {
        delete [] body;
        delete [] bigbody;
    }

    T * operator()(size_t line, size_t col) {
        return body[line]+col;
    }

    T * operator()(size_t line, size_t col) const {
        return body[line]+col;
    }


    bool mul(blockmatrix const &left, blockmatrix const &right) {
        if (left.columns != right.lines) return false;
        if (lines != left.lines) return false;
        if (columns != right.columns) return false;
        for (size_t i = 0; i < lines; i++) {
            for (size_t j = 0; j < columns; j++) {
                T acc = (T) 0;
                for (size_t k = 0; k < left.columns; k++) {
                    acc += *left(i,k) * *right(k, j);
                }
                *(*this)(i,j) = acc;
            }
        }
        return true;
    }

    bool better_mul(blockmatrix const &left, blockmatrix const &right) 
    {
        if (left.columns != right.lines) 
			return false;
        if (lines != left.lines) 
			return false;
        if (columns != right.columns) 
			return false;

		//params for future computations
        int n = left.lines;
        int m = left.columns;
        int p = right.columns;

        if(n <= cash && m <= cash && p <= cash)
        {  // if size of matrix small enought to be
			//fully inserted into the cash, then just
            //transposing and multiplying it
            blockmatrix right_t(right.columns, right.lines);
            for (int i = 0; i < right.lines; i++)
            {
                T* r_p = right(i,0);
                for (int j = 0; j < right.columns; j++) 
                {
                    *right_t(j, i) = *r_p;
                    r_p++;
                }
            }

            T* f_p = (*this)(0, 0);
            for (size_t i = 0; i < lines; i++) 
            {
                for (size_t j = 0; j < columns; j++) 
                {
                    T acc = (T) 0;
                    T* l_p = left(i,0);
                    T* r_p = right_t(j,0);

                    for (size_t k = 0; k < left.columns; k++) 
                    {
                        acc += *l_p * *r_p;
                        l_p++;
                        r_p++;
                    }
                    *f_p++ = acc;
                }
            }
            return true;
        }

        if(n >= m && n >= p)
        { // if matrix actually bigger, then we split it
			//according to the direction along witch it has the biggest size
            blockmatrix top(n/2, m);//2 ---> 4 matrix
            blockmatrix bot(n/2, m);
            blockmatrix res1(n/2, p);
            blockmatrix res2(n/2, p);

            top.podmatrix(left, 0, 0);
            bot.podmatrix(left, n/2, 0);
            res1.better_mul(top, right);
            res2.better_mul(bot, right);
            this->replace(res1, 0, 0);
            this->replace(res2, n/2, 0);
            return true;
        }
        else if(p >= m && p >= n)
        { 
            blockmatrix lft(m, p/2);
            blockmatrix rght(m, p/2);
            blockmatrix res1(n, p/2);
            blockmatrix res2(n, p/2);
            
            lft.podmatrix(right, 0, 0);
            rght.podmatrix(right, 0, p/2);
            res1.better_mul(left, lft);
            res2.better_mul(left, rght);
            this->replace(res1, 0, 0);
            this->replace(res2, 0, p/2);
            return true;

        }
        else if(m >= n && m >= p)
        {
            blockmatrix lft(n, m/2);
            blockmatrix rght(n, m/2);
            blockmatrix top(m/2, p);
            blockmatrix bottom(m/2, p);
            blockmatrix res1(n, p);
            blockmatrix res2(n, p);

            top.podmatrix(right, 0, 0);
            bottom.podmatrix(right, m/2, 0);
            lft.podmatrix(left, 0, 0);
            rght.podmatrix(left, 0, m/2);

            res1.better_mul(lft, top);
            res2.better_mul(rght, bottom);

            this->add(res1, res2);
            return true;
        }
        printf("Wrong matrix sizes\n");
        return false;
    }
	bool just_transpose_mul(blockmatrix const &left, blockmatrix const &right) 
    {
        if (left.columns != right.lines) 
			return false;
        if (lines != left.lines) 
			return false;
        if (columns != right.columns) 
			return false;

        int n = left.lines;
        int m = left.columns;
        int p = right.columns;


		blockmatrix right_t(right.columns, right.lines);
		for (int i = 0; i < right.lines; i++)
		{
			T* r_p = right(i,0);
			for (int j = 0; j < right.columns; j++) 
			{
				*right_t(j, i) = *r_p;
				r_p++;
			}
		}

		T* f_p = (*this)(0, 0);
		for (size_t i = 0; i < lines; i++) 
		{
			for (size_t j = 0; j < columns; j++) 
			{
				T acc = (T) 0;
				T* l_p = left(i,0);
				T* r_p = right_t(j,0);

				for (size_t k = 0; k < left.columns; k++) 
				{
					acc += *l_p * *r_p;
					l_p++;
					r_p++;
				}
				*f_p++ = acc;
			}
		}
		return true;
	}
    bool add(blockmatrix const &left, blockmatrix const &right)
    {
        for (int i = 0; i < right.lines; i++)
        {
            T* f_p = (*this)(i, 0);
            T* l_p = left(i,0);
            T* r_p = right(i,0);
            for (int j = 0; j < right.columns; j++) 
            {
                *f_p = *r_p + *l_p;
                l_p++;
                r_p++;
                f_p++;
            }
        }
        return true;
    }

private:
    T ** body;
    T *bigbody;
    size_t lines, columns;

    void podmatrix(blockmatrix const &init, int i_s, int j_s)
    {
        for(int i = 0; i < lines; i++)
        {
            T* f_p = (*this)(i, 0);
            T* p_p = init(i_s + i, j_s);
            for(int j = 0; j < columns; j++)
            {
                *f_p = *p_p;
                f_p++;
                p_p++;
            }
        }
    }
    void replace(blockmatrix const &init, int i_s, int j_s)
    {
        for(int i = 0; i < init.lines; i++)
        {
            T* f_p = (*this)(i_s + i, j_s);
            T* p_p = init(i, 0);
            for(int j = 0; j < init.columns; j++)
            {
                *f_p = *p_p;
                f_p++;
                p_p++;
            }
        }
    }
};

