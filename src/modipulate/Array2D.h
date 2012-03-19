// Copyright (c) 2009, Eric Gregory
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//    * Neither the name of Eric Gregory nor the
//      names of its contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY ERIC GREGORY ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL ERIC GREGORY  BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef ARRAY2D_H_
#define ARRAY2D_H_

template <class T>
class Array2D
{
public:

	Array2D()
	{
		data = NULL;
		width = 0;
		height = 0;
	}

	Array2D( const int& width, const int& height )
	{
		data = NULL;
		allocate( width, height );
	}

	~Array2D()
	{
		deallocate();
	}

	T get( const int& x, const int& y )
	{
		return data[y][x];
	}

	void set( const int& x, const int& y, const T& t )
	{
		data[y][x] = t;
	}

	void setAll( const T& t )
	{
		for ( int y = 0; y < height; y++ )
		{
			for ( int x = 0; x < width; x++ )
			{
				data[y][x] = t;
			}
		}
	}

	int getWidth()
	{
		return width;
	}

	int getHeight()
	{
		return height;
	}

private:

	void allocate( const int& width, const int& height )
	{
		// Remember dimensions.
		this->width = width;
		this->height = height;

		// Allocate.
		data = new T*[height];
		for ( int i = 0; i < height; i++ )
		{
			data[i] = new T[width];
		}
	}

	void deallocate()
	{
		if ( NULL == data )
		{
			// Nothing to do.
			return;
		}

		// Free the memory.
		for ( int i = 0; i < height; i++ )
		{
			delete[] data[i];
		}
		delete[] data;

		// Reset.
		width = 0;
		height = 0;
		data = NULL;
	}

	int width;
	int height;
	T** data;
};

#endif /* ARRAY2D_H_ */