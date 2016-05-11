//======================================================================================
/** \file satmath.h
 *    This file contains a very simple saturated math library
 *
 *  Revisions:
 *    \li 01-14-2015 CTR Original File
 *    \li 10-13-2015 CTR Original file (.c and .h files)
 *    \li 01-19-2016 CTR Modified for C++
 *    \li 01-27-2016 Added doxygen comments
 *
 *  License:
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 *    TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 *    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 *    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 *    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//======================================================================================

// This define prevents this .H file from being included multiple times in a .CPP file
#ifndef SATMATH_H
#define SATMATH_H

// If you dont want to use C++11 then you made need to uncomment these lines
//#define INT16_MIN (int16_t) 0x8000
//#define INT16_MAX (int16_t) 0x7FFF
//#define INT32_MIN (int32_t) 0x80000000
//#define INT32_MAX (int32_t) 0x7FFFFFFF

// Some handy macros to shorten the function names
/** \brief Shorthand to make accessing namespace easier
 */
#define ssadd(x,y)	satmath::signed_saturated_add(x,y)

/** \brief Shorthand to make accessing namespace easier
 */
#define sssub(x,y)	satmath::signed_saturated_sub(x,y)

/** \brief Shorthand to make accessing namespace easier
 */
#define ssabs(x)	satmath::saturated_abs(x)

/** \brief Shorthand to make accessing namespace easier
 */
#define ssmul(x,y)	satmath::signed_saturated_mul(x,y)

/** \brief Shorthand to make accessing namespace easier
 */
#define ssdiv(x,y)	satmath::signed_saturated_div(x,y)

//-------------------------------------------------------------------------------------
/** \brief This namespace includes several functions to do saturated 16-bit signed math
 */
namespace satmath 
{
	int16_t				signed_saturated_add(int16_t x, int16_t y);
	int16_t				signed_saturated_sub(int16_t x, int16_t y);
	int16_t				saturated_abs(int16_t x);
	int32_t				signed_saturated_mul(int16_t x, int16_t y);
	int16_t				signed_saturated_div(int32_t x, int16_t y);	
} // end namespace satmath

#endif // SATMATH_H