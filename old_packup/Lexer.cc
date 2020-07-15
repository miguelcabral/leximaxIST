/******************************************************************************\
 *    This file is part of packup.                                            *
 *                                                                            *
 *    packup is free software: you can redistribute it and/or modify          *
 *    it under the terms of the GNU General Public License as published by    *
 *    the Free Software Foundation, either version 3 of the License, or       *
 *    (at your option) any later version.                                     *
 *                                                                            *
 *    packup is distributed in the hope that it will be useful,               *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 *    GNU General Public License for more details.                            *
 *                                                                            *
 *    You should have received a copy of the GNU General Public License       *
 *    along with packup.  If not, see <http://www.gnu.org/licenses/>.         *            
\******************************************************************************/           

#line 1 "l.rl"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
using namespace std;
using std::cerr;
using std::cout;
using std::cin;
using std::endl;

#include "Lexer.hh"
#include "p.tab.hh"
extern YYSTYPE yylval;

char* stop;
char* token_start;
int token_length;

#define EMIT(t)  { token = t; token_start=ts; token_length =te-ts;  stop =p+1; token_read = true;}


#line 152 "l.rl"



#line 30 "Lexer.cc"
static const char _cud_actions[] = {
	0, 1, 1, 1, 2, 1, 4, 1, 
	5, 1, 6, 1, 7, 1, 8, 1, 
	9, 1, 10, 1, 11, 1, 12, 1, 
	14, 1, 15, 1, 16, 1, 17, 1, 
	18, 1, 19, 1, 23, 1, 24, 1, 
	25, 1, 26, 1, 27, 1, 28, 1, 
	29, 1, 30, 1, 31, 1, 32, 1, 
	33, 1, 34, 1, 35, 1, 36, 1, 
	37, 1, 38, 1, 39, 1, 40, 1, 
	41, 1, 42, 1, 43, 1, 44, 1, 
	45, 1, 46, 1, 47, 1, 48, 1, 
	49, 1, 50, 1, 51, 1, 52, 1, 
	53, 1, 54, 1, 55, 1, 56, 1, 
	57, 1, 58, 1, 59, 1, 60, 2, 
	0, 13, 2, 0, 16, 2, 0, 23, 
	2, 2, 3, 2, 5, 0, 2, 5, 
	20, 2, 5, 21, 2, 5, 22
};

static const short _cud_key_offsets[] = {
	0, 0, 1, 7, 14, 21, 28, 35, 
	42, 49, 56, 63, 69, 76, 83, 90, 
	97, 104, 111, 117, 124, 131, 138, 145, 
	152, 159, 166, 173, 179, 186, 193, 200, 
	206, 214, 221, 228, 235, 242, 249, 255, 
	263, 270, 277, 284, 291, 298, 304, 311, 
	318, 325, 332, 339, 345, 352, 361, 368, 
	375, 382, 389, 396, 403, 410, 416, 423, 
	430, 437, 443, 450, 457, 464, 471, 477, 
	484, 491, 498, 505, 512, 519, 525, 532, 
	539, 546, 553, 560, 567, 573, 574, 575, 
	576, 577, 578, 579, 580, 581, 582, 583, 
	584, 585, 586, 587, 588, 589, 590, 591, 
	592, 593, 594, 595, 596, 597, 598, 599, 
	600, 601, 602, 603, 605, 607, 621, 623, 
	624, 625, 626, 627, 628, 629, 630, 631, 
	632, 633, 634, 635, 636, 637, 638, 645, 
	648, 649, 650, 651, 656, 657, 685, 686, 
	688, 698, 710, 711, 712, 725, 739, 753, 
	767, 781, 795, 809, 823, 837
};

static const char _cud_trans_keys[] = {
	10, 45, 58, 48, 57, 97, 122, 45, 
	58, 111, 48, 57, 97, 122, 45, 58, 
	110, 48, 57, 97, 122, 45, 58, 102, 
	48, 57, 97, 122, 45, 58, 108, 48, 
	57, 97, 122, 45, 58, 105, 48, 57, 
	97, 122, 45, 58, 99, 48, 57, 97, 
	122, 45, 58, 116, 48, 57, 97, 122, 
	45, 58, 115, 48, 57, 97, 122, 45, 
	58, 48, 57, 97, 122, 45, 58, 101, 
	48, 57, 97, 122, 45, 58, 112, 48, 
	57, 97, 122, 45, 58, 101, 48, 57, 
	97, 122, 45, 58, 110, 48, 57, 97, 
	122, 45, 58, 100, 48, 57, 97, 122, 
	45, 58, 115, 48, 57, 97, 122, 45, 
	58, 48, 57, 97, 122, 45, 58, 110, 
	48, 57, 97, 122, 45, 58, 115, 48, 
	57, 97, 122, 45, 58, 116, 48, 57, 
	97, 122, 45, 58, 97, 48, 57, 98, 
	122, 45, 58, 108, 48, 57, 97, 122, 
	45, 58, 108, 48, 57, 97, 122, 45, 
	58, 101, 48, 57, 97, 122, 45, 58, 
	100, 48, 57, 97, 122, 45, 58, 48, 
	57, 97, 122, 45, 58, 101, 48, 57, 
	97, 122, 45, 58, 101, 48, 57, 97, 
	122, 45, 58, 112, 48, 57, 97, 122, 
	45, 58, 48, 57, 97, 122, 45, 58, 
	97, 114, 48, 57, 98, 122, 45, 58, 
	99, 48, 57, 97, 122, 45, 58, 107, 
	48, 57, 97, 122, 45, 58, 97, 48, 
	57, 98, 122, 45, 58, 103, 48, 57, 
	97, 122, 45, 58, 101, 48, 57, 97, 
	122, 45, 58, 48, 57, 97, 122, 45, 
	58, 101, 111, 48, 57, 97, 122, 45, 
	58, 97, 48, 57, 98, 122, 45, 58, 
	109, 48, 57, 97, 122, 45, 58, 98, 
	48, 57, 97, 122, 45, 58, 108, 48, 
	57, 97, 122, 45, 58, 101, 48, 57, 
	97, 122, 45, 58, 48, 57, 97, 122, 
	45, 58, 118, 48, 57, 97, 122, 45, 
	58, 105, 48, 57, 97, 122, 45, 58, 
	100, 48, 57, 97, 122, 45, 58, 101, 
	48, 57, 97, 122, 45, 58, 115, 48, 
	57, 97, 122, 45, 58, 48, 57, 97, 
	122, 45, 58, 101, 48, 57, 97, 122, 
	45, 58, 99, 109, 113, 48, 57, 97, 
	122, 45, 58, 111, 48, 57, 97, 122, 
	45, 58, 109, 48, 57, 97, 122, 45, 
	58, 109, 48, 57, 97, 122, 45, 58, 
	101, 48, 57, 97, 122, 45, 58, 110, 
	48, 57, 97, 122, 45, 58, 100, 48, 
	57, 97, 122, 45, 58, 115, 48, 57, 
	97, 122, 45, 58, 48, 57, 97, 122, 
	45, 58, 111, 48, 57, 97, 122, 45, 
	58, 118, 48, 57, 97, 122, 45, 58, 
	101, 48, 57, 97, 122, 45, 58, 48, 
	57, 97, 122, 45, 58, 117, 48, 57, 
	97, 122, 45, 58, 101, 48, 57, 97, 
	122, 45, 58, 115, 48, 57, 97, 122, 
	45, 58, 116, 48, 57, 97, 122, 45, 
	58, 48, 57, 97, 122, 45, 58, 112, 
	48, 57, 97, 122, 45, 58, 103, 48, 
	57, 97, 122, 45, 58, 114, 48, 57, 
	97, 122, 45, 58, 97, 48, 57, 98, 
	122, 45, 58, 100, 48, 57, 97, 122, 
	45, 58, 101, 48, 57, 97, 122, 45, 
	58, 48, 57, 97, 122, 45, 58, 101, 
	48, 57, 97, 122, 45, 58, 114, 48, 
	57, 97, 122, 45, 58, 115, 48, 57, 
	97, 122, 45, 58, 105, 48, 57, 97, 
	122, 45, 58, 111, 48, 57, 97, 122, 
	45, 58, 110, 48, 57, 97, 122, 45, 
	58, 48, 57, 97, 122, 32, 101, 97, 
	116, 117, 114, 101, 111, 110, 101, 97, 
	99, 107, 97, 103, 101, 101, 114, 115, 
	105, 111, 110, 97, 108, 115, 101, 114, 
	117, 101, 61, 34, 92, 34, 92, 9, 
	10, 32, 35, 99, 100, 105, 107, 112, 
	114, 117, 118, 97, 122, 9, 32, 10, 
	32, 32, 32, 32, 32, 32, 10, 32, 
	32, 32, 32, 10, 32, 32, 9, 10, 
	32, 102, 110, 112, 118, 9, 10, 32, 
	32, 10, 32, 9, 10, 32, 102, 116, 
	32, 9, 10, 32, 33, 34, 37, 44, 
	58, 60, 61, 62, 91, 93, 102, 116, 
	124, 40, 41, 43, 45, 46, 47, 48, 
	57, 64, 90, 97, 122, 32, 34, 92, 
	37, 43, 40, 41, 45, 57, 64, 90, 
	97, 122, 37, 43, 40, 41, 45, 47, 
	48, 57, 64, 90, 97, 122, 61, 61, 
	37, 43, 45, 40, 41, 46, 47, 48, 
	57, 64, 90, 97, 122, 37, 43, 45, 
	97, 40, 41, 46, 47, 48, 57, 64, 
	90, 98, 122, 37, 43, 45, 108, 40, 
	41, 46, 47, 48, 57, 64, 90, 97, 
	122, 37, 43, 45, 115, 40, 41, 46, 
	47, 48, 57, 64, 90, 97, 122, 37, 
	43, 45, 101, 40, 41, 46, 47, 48, 
	57, 64, 90, 97, 122, 33, 37, 43, 
	45, 40, 41, 46, 47, 48, 57, 64, 
	90, 97, 122, 37, 43, 45, 114, 40, 
	41, 46, 47, 48, 57, 64, 90, 97, 
	122, 37, 43, 45, 117, 40, 41, 46, 
	47, 48, 57, 64, 90, 97, 122, 37, 
	43, 45, 101, 40, 41, 46, 47, 48, 
	57, 64, 90, 97, 122, 33, 37, 43, 
	45, 40, 41, 46, 47, 48, 57, 64, 
	90, 97, 122, 0
};

static const char _cud_single_lengths[] = {
	0, 1, 2, 3, 3, 3, 3, 3, 
	3, 3, 3, 2, 3, 3, 3, 3, 
	3, 3, 2, 3, 3, 3, 3, 3, 
	3, 3, 3, 2, 3, 3, 3, 2, 
	4, 3, 3, 3, 3, 3, 2, 4, 
	3, 3, 3, 3, 3, 2, 3, 3, 
	3, 3, 3, 2, 3, 5, 3, 3, 
	3, 3, 3, 3, 3, 2, 3, 3, 
	3, 2, 3, 3, 3, 3, 2, 3, 
	3, 3, 3, 3, 3, 2, 3, 3, 
	3, 3, 3, 3, 2, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 2, 2, 12, 2, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 7, 3, 
	1, 1, 1, 5, 1, 16, 1, 2, 
	2, 2, 1, 1, 3, 4, 4, 4, 
	4, 4, 4, 4, 4, 4
};

static const char _cud_range_lengths[] = {
	0, 0, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 2, 2, 2, 
	2, 2, 2, 2, 2, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 1, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 6, 0, 0, 
	4, 5, 0, 0, 5, 5, 5, 5, 
	5, 5, 5, 5, 5, 5
};

static const short _cud_index_offsets[] = {
	0, 0, 2, 7, 13, 19, 25, 31, 
	37, 43, 49, 55, 60, 66, 72, 78, 
	84, 90, 96, 101, 107, 113, 119, 125, 
	131, 137, 143, 149, 154, 160, 166, 172, 
	177, 184, 190, 196, 202, 208, 214, 219, 
	226, 232, 238, 244, 250, 256, 261, 267, 
	273, 279, 285, 291, 296, 302, 310, 316, 
	322, 328, 334, 340, 346, 352, 357, 363, 
	369, 375, 380, 386, 392, 398, 404, 409, 
	415, 421, 427, 433, 439, 445, 450, 456, 
	462, 468, 474, 480, 486, 491, 493, 495, 
	497, 499, 501, 503, 505, 507, 509, 511, 
	513, 515, 517, 519, 521, 523, 525, 527, 
	529, 531, 533, 535, 537, 539, 541, 543, 
	545, 547, 549, 551, 554, 557, 571, 574, 
	576, 578, 580, 582, 584, 586, 588, 590, 
	592, 594, 596, 598, 600, 602, 604, 612, 
	616, 618, 620, 622, 628, 630, 653, 655, 
	658, 665, 673, 675, 677, 686, 696, 706, 
	716, 726, 736, 746, 756, 766
};

static const unsigned char _cud_indicies[] = {
	1, 0, 2, 4, 2, 2, 3, 2, 
	4, 5, 2, 2, 3, 2, 4, 6, 
	2, 2, 3, 2, 4, 7, 2, 2, 
	3, 2, 4, 8, 2, 2, 3, 2, 
	4, 9, 2, 2, 3, 2, 4, 10, 
	2, 2, 3, 2, 4, 11, 2, 2, 
	3, 2, 4, 12, 2, 2, 3, 2, 
	13, 2, 2, 3, 2, 4, 14, 2, 
	2, 3, 2, 4, 15, 2, 2, 3, 
	2, 4, 16, 2, 2, 3, 2, 4, 
	17, 2, 2, 3, 2, 4, 18, 2, 
	2, 3, 2, 4, 19, 2, 2, 3, 
	2, 20, 2, 2, 3, 2, 4, 21, 
	2, 2, 3, 2, 4, 22, 2, 2, 
	3, 2, 4, 23, 2, 2, 3, 2, 
	4, 24, 2, 2, 3, 2, 4, 25, 
	2, 2, 3, 2, 4, 26, 2, 2, 
	3, 2, 27, 28, 2, 2, 3, 2, 
	4, 29, 2, 2, 3, 2, 30, 2, 
	2, 3, 2, 4, 31, 2, 2, 3, 
	2, 4, 32, 2, 2, 3, 2, 4, 
	33, 2, 2, 3, 2, 34, 2, 2, 
	3, 2, 4, 35, 36, 2, 2, 3, 
	2, 4, 37, 2, 2, 3, 2, 4, 
	38, 2, 2, 3, 2, 4, 39, 2, 
	2, 3, 2, 4, 40, 2, 2, 3, 
	2, 4, 41, 2, 2, 3, 2, 42, 
	2, 2, 3, 2, 4, 43, 44, 2, 
	2, 3, 2, 4, 45, 2, 2, 3, 
	2, 4, 46, 2, 2, 3, 2, 4, 
	47, 2, 2, 3, 2, 4, 48, 2, 
	2, 3, 2, 4, 49, 2, 2, 3, 
	2, 50, 2, 2, 3, 2, 4, 51, 
	2, 2, 3, 2, 4, 52, 2, 2, 
	3, 2, 4, 53, 2, 2, 3, 2, 
	4, 54, 2, 2, 3, 2, 4, 55, 
	2, 2, 3, 2, 56, 2, 2, 3, 
	2, 4, 57, 2, 2, 3, 2, 4, 
	58, 59, 60, 2, 2, 3, 2, 4, 
	61, 2, 2, 3, 2, 4, 62, 2, 
	2, 3, 2, 4, 63, 2, 2, 3, 
	2, 4, 64, 2, 2, 3, 2, 4, 
	65, 2, 2, 3, 2, 4, 66, 2, 
	2, 3, 2, 4, 67, 2, 2, 3, 
	2, 68, 2, 2, 3, 2, 4, 69, 
	2, 2, 3, 2, 4, 70, 2, 2, 
	3, 2, 4, 71, 2, 2, 3, 2, 
	72, 2, 2, 3, 2, 4, 73, 2, 
	2, 3, 2, 4, 74, 2, 2, 3, 
	2, 4, 75, 2, 2, 3, 2, 4, 
	76, 2, 2, 3, 2, 77, 2, 2, 
	3, 2, 4, 78, 2, 2, 3, 2, 
	4, 79, 2, 2, 3, 2, 4, 80, 
	2, 2, 3, 2, 4, 81, 2, 2, 
	3, 2, 4, 82, 2, 2, 3, 2, 
	4, 83, 2, 2, 3, 2, 84, 2, 
	2, 3, 2, 4, 85, 2, 2, 3, 
	2, 4, 86, 2, 2, 3, 2, 4, 
	87, 2, 2, 3, 2, 4, 88, 2, 
	2, 3, 2, 4, 89, 2, 2, 3, 
	2, 4, 90, 2, 2, 3, 2, 91, 
	2, 2, 3, 93, 92, 94, 3, 95, 
	3, 96, 3, 97, 3, 98, 3, 99, 
	3, 100, 3, 101, 3, 102, 3, 103, 
	3, 104, 3, 105, 3, 106, 3, 107, 
	3, 108, 3, 109, 3, 110, 3, 111, 
	3, 112, 3, 113, 3, 114, 3, 115, 
	3, 116, 3, 117, 3, 118, 3, 119, 
	3, 120, 3, 121, 3, 122, 3, 125, 
	126, 124, 127, 126, 124, 128, 129, 128, 
	0, 130, 131, 132, 133, 134, 135, 136, 
	137, 2, 3, 128, 128, 138, 129, 139, 
	141, 140, 142, 140, 143, 140, 144, 140, 
	145, 140, 146, 140, 147, 50, 148, 140, 
	149, 140, 150, 140, 151, 140, 152, 151, 
	153, 140, 154, 140, 155, 156, 155, 157, 
	158, 159, 160, 3, 155, 162, 155, 161, 
	93, 163, 165, 164, 167, 166, 168, 169, 
	168, 170, 171, 3, 173, 172, 174, 175, 
	174, 176, 124, 177, 179, 181, 182, 183, 
	184, 185, 186, 188, 189, 190, 177, 178, 
	177, 180, 177, 187, 3, 192, 191, 125, 
	126, 124, 177, 177, 177, 177, 177, 177, 
	194, 177, 177, 177, 177, 180, 177, 177, 
	123, 196, 195, 198, 197, 177, 177, 187, 
	177, 177, 187, 177, 187, 199, 177, 177, 
	187, 200, 177, 177, 187, 177, 187, 199, 
	177, 177, 187, 201, 177, 177, 187, 177, 
	187, 199, 177, 177, 187, 202, 177, 177, 
	187, 177, 187, 199, 177, 177, 187, 203, 
	177, 177, 187, 177, 187, 199, 204, 177, 
	177, 187, 177, 177, 187, 177, 187, 199, 
	177, 177, 187, 205, 177, 177, 187, 177, 
	187, 199, 177, 177, 187, 206, 177, 177, 
	187, 177, 187, 199, 177, 177, 187, 207, 
	177, 177, 187, 177, 187, 199, 208, 177, 
	177, 187, 177, 177, 187, 177, 187, 199, 
	0
};

static const unsigned char _cud_trans_targs[] = {
	1, 117, 2, 0, 117, 4, 5, 6, 
	7, 8, 9, 10, 11, 120, 13, 14, 
	15, 16, 17, 18, 121, 20, 21, 22, 
	23, 24, 25, 122, 26, 27, 123, 29, 
	30, 31, 124, 33, 39, 34, 35, 36, 
	37, 38, 125, 40, 46, 41, 42, 43, 
	44, 45, 126, 47, 48, 49, 50, 51, 
	127, 53, 54, 62, 66, 55, 56, 57, 
	58, 59, 60, 61, 128, 63, 64, 65, 
	129, 67, 68, 69, 70, 130, 72, 73, 
	74, 75, 76, 77, 132, 79, 80, 81, 
	82, 83, 84, 133, 134, 135, 87, 88, 
	89, 90, 91, 134, 93, 94, 134, 96, 
	97, 98, 99, 100, 134, 102, 103, 104, 
	105, 106, 134, 108, 109, 110, 139, 112, 
	113, 139, 141, 141, 115, 141, 116, 143, 
	118, 119, 3, 12, 19, 28, 32, 52, 
	71, 78, 117, 117, 117, 117, 117, 117, 
	117, 117, 117, 117, 117, 117, 117, 131, 
	117, 117, 117, 135, 136, 86, 92, 95, 
	101, 134, 85, 134, 137, 138, 137, 137, 
	139, 140, 107, 111, 139, 139, 141, 142, 
	114, 144, 145, 141, 145, 141, 146, 141, 
	147, 141, 141, 148, 149, 154, 141, 141, 
	141, 141, 141, 141, 141, 141, 141, 141, 
	150, 151, 152, 153, 141, 155, 156, 157, 
	141
};

static const unsigned char _cud_trans_actions[] = {
	0, 75, 0, 0, 99, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 21, 123, 0, 0, 
	0, 0, 0, 15, 0, 0, 9, 0, 
	0, 0, 0, 0, 13, 0, 0, 0, 
	0, 0, 11, 0, 0, 0, 31, 0, 
	0, 29, 51, 73, 0, 37, 0, 126, 
	0, 1, 0, 0, 0, 0, 0, 0, 
	0, 0, 101, 103, 109, 85, 83, 91, 
	87, 77, 79, 105, 89, 97, 93, 0, 
	107, 95, 81, 7, 1, 0, 0, 0, 
	0, 17, 0, 19, 23, 1, 25, 111, 
	27, 1, 0, 0, 33, 114, 35, 1, 
	0, 0, 132, 43, 129, 45, 0, 49, 
	0, 39, 41, 0, 0, 0, 47, 61, 
	117, 63, 71, 67, 55, 65, 53, 69, 
	0, 0, 0, 0, 59, 0, 0, 0, 
	57
};

static const unsigned char _cud_to_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 3, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 3, 0, 
	0, 3, 0, 3, 0, 120, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0
};

static const unsigned char _cud_from_state_actions[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 5, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 5, 0, 
	0, 5, 0, 5, 0, 5, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0
};

static const short _cud_eof_trans[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 93, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 124, 124, 0, 139, 140, 
	141, 141, 141, 141, 141, 141, 148, 141, 
	141, 141, 141, 153, 141, 141, 0, 162, 
	164, 0, 167, 0, 173, 0, 192, 194, 
	195, 124, 196, 198, 200, 200, 200, 200, 
	200, 200, 200, 200, 200, 200
};

static const int cud_start = 117;
static const int cud_error = 0;

static const int cud_en_keep_line = 134;
static const int cud_en_skip_line = 137;
static const int cud_en_bool_line = 139;
static const int cud_en_middle_line = 141;
static const int cud_en_main = 117;


#line 155 "l.rl"

char* Lexer::mkstring(const char* ts, unsigned int length)
{
     char* return_value = new char[length+1];
     return_value[length]='\0';
     memcpy(return_value, ts, length);
     return return_value;
 }

Lexer::Lexer(istream& input_stream) : input(input_stream)
{
    current_line = 1;
    token = -1;
    done = false;
    token_read = false;

    should_read = true;
    last_buffer = false;

    // Ragel variables
    cs=act=have = 0;
    ts=te = 0;
    eof = 0;

    std::ios::sync_with_stdio(false);
    
#line 520 "Lexer.cc"
	{
	cs = cud_start;
	ts = 0;
	te = 0;
	act = 0;
	}

#line 181 "l.rl"
}

    int Lexer::get_current_line() { return current_line;}

    int Lexer::yylex_internal() {
        if (done) return LEXER_END;
        if (should_read) {
            should_read = false;
            p = buf + have;
            int space = BUFSIZE - have;

            if (space == 0) {
                /* We filled up the buffer trying to scan a token. */
                cerr << "OUT OF BUFFER SPACE" << endl;
                exit(1);
            }

            input.read(p, space);
            int len = input.gcount();
//fwrite(p, 1, len, stdout );
            pe = p + len;
            /* If we see eof then append the EOF char. */
            if (input.eof()) {
                last_buffer = true;
                eof = pe;
            } else eof = 0;
        }

        // Run automaton
        
#line 559 "Lexer.cc"
	{
	int _klen;
	unsigned int _trans;
	const char *_acts;
	unsigned int _nacts;
	const char *_keys;

	if ( p == pe )
		goto _test_eof;
	if ( cs == 0 )
		goto _out;
_resume:
	_acts = _cud_actions + _cud_from_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 4:
#line 1 "NONE"
	{ts = p;}
	break;
#line 580 "Lexer.cc"
		}
	}

	_keys = _cud_trans_keys + _cud_key_offsets[cs];
	_trans = _cud_index_offsets[cs];

	_klen = _cud_single_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + _klen - 1;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + ((_upper-_lower) >> 1);
			if ( (*p) < *_mid )
				_upper = _mid - 1;
			else if ( (*p) > *_mid )
				_lower = _mid + 1;
			else {
				_trans += (_mid - _keys);
				goto _match;
			}
		}
		_keys += _klen;
		_trans += _klen;
	}

	_klen = _cud_range_lengths[cs];
	if ( _klen > 0 ) {
		const char *_lower = _keys;
		const char *_mid;
		const char *_upper = _keys + (_klen<<1) - 2;
		while (1) {
			if ( _upper < _lower )
				break;

			_mid = _lower + (((_upper-_lower) >> 1) & ~1);
			if ( (*p) < _mid[0] )
				_upper = _mid - 2;
			else if ( (*p) > _mid[1] )
				_lower = _mid + 2;
			else {
				_trans += ((_mid - _keys)>>1);
				goto _match;
			}
		}
		_trans += _klen;
	}

_match:
	_trans = _cud_indicies[_trans];
_eof_trans:
	cs = _cud_trans_targs[_trans];

	if ( _cud_trans_actions[_trans] == 0 )
		goto _again;

	_acts = _cud_actions + _cud_trans_actions[_trans];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 )
	{
		switch ( *_acts++ )
		{
	case 0:
#line 24 "l.rl"
	{current_line += 1;}
	break;
	case 1:
#line 25 "l.rl"
	{current_line += 1;}
	break;
	case 5:
#line 1 "NONE"
	{te = p+1;}
	break;
	case 6:
#line 63 "l.rl"
	{te = p+1;{ EMIT (KEEP_NONE_TOKEN); {p++; goto _out; }  }}
	break;
	case 7:
#line 64 "l.rl"
	{te = p+1;{ EMIT (KEEP_VERSION_TOKEN); {p++; goto _out; }  }}
	break;
	case 8:
#line 65 "l.rl"
	{te = p+1;{ EMIT (KEEP_PACKAGE_TOKEN); {p++; goto _out; }  }}
	break;
	case 9:
#line 66 "l.rl"
	{te = p+1;{ EMIT (KEEP_FEATURE_TOKEN); {p++; goto _out; }  }}
	break;
	case 10:
#line 62 "l.rl"
	{te = p;p--;}
	break;
	case 11:
#line 67 "l.rl"
	{te = p;p--;{ EMIT (NEW_LINE); cs = 117; {p++; goto _out; }  }}
	break;
	case 12:
#line 62 "l.rl"
	{{p = ((te))-1;}}
	break;
	case 13:
#line 71 "l.rl"
	{te = p+1;}
	break;
	case 14:
#line 73 "l.rl"
	{te = p+1;}
	break;
	case 15:
#line 72 "l.rl"
	{te = p;p--;{ {cs = 117; goto _again;} }}
	break;
	case 16:
#line 77 "l.rl"
	{te = p+1;}
	break;
	case 17:
#line 80 "l.rl"
	{te = p+1;{ EMIT(BOOL_TRUE); {p++; goto _out; }}}
	break;
	case 18:
#line 81 "l.rl"
	{te = p+1;{ EMIT(BOOL_FALSE); {p++; goto _out; }}}
	break;
	case 19:
#line 79 "l.rl"
	{te = p;p--;{ cs = 117; EMIT(NEW_LINE); {p++; goto _out; }}}
	break;
	case 20:
#line 89 "l.rl"
	{act = 18;}
	break;
	case 21:
#line 113 "l.rl"
	{act = 32;}
	break;
	case 22:
#line 115 "l.rl"
	{act = 34;}
	break;
	case 23:
#line 85 "l.rl"
	{te = p+1;}
	break;
	case 24:
#line 89 "l.rl"
	{te = p+1;{
		    EMIT(STRING_VALUE); 
//			fwrite( token_start, 1, token_length, stdout );
//			printf ("\n");	
			{p++; goto _out; }
		}}
	break;
	case 25:
#line 96 "l.rl"
	{te = p+1;{ EMIT(OPEN_SQUARE); {p++; goto _out; }}}
	break;
	case 26:
#line 97 "l.rl"
	{te = p+1;{ EMIT(CLOSE_SQUARE); {p++; goto _out; }}}
	break;
	case 27:
#line 98 "l.rl"
	{te = p+1;{ EMIT(COMMA); {p++; goto _out; }}}
	break;
	case 28:
#line 99 "l.rl"
	{te = p+1;{ EMIT(COLON); {p++; goto _out; }}}
	break;
	case 29:
#line 100 "l.rl"
	{te = p+1;{EMIT(PIPE); {p++; goto _out; }}}
	break;
	case 30:
#line 102 "l.rl"
	{te = p+1;{EMIT (EQUALS); {p++; goto _out; }}}
	break;
	case 31:
#line 103 "l.rl"
	{te = p+1;{EMIT (NOT_EQUALS); {p++; goto _out; }}}
	break;
	case 32:
#line 104 "l.rl"
	{te = p+1;{EMIT (GREATER_EQUALS); {p++; goto _out; }}}
	break;
	case 33:
#line 106 "l.rl"
	{te = p+1;{EMIT (LESS_EQUALS); {p++; goto _out; }}}
	break;
	case 34:
#line 109 "l.rl"
	{te = p+1;{ EMIT(TRUE_BANG); {p++; goto _out; }}}
	break;
	case 35:
#line 110 "l.rl"
	{te = p+1;{ EMIT(FALSE_BANG); {p++; goto _out; }}}
	break;
	case 36:
#line 87 "l.rl"
	{te = p;p--;{ cs = 117; EMIT(NEW_LINE); {p++; goto _out; }}}
	break;
	case 37:
#line 89 "l.rl"
	{te = p;p--;{
		    EMIT(STRING_VALUE); 
//			fwrite( token_start, 1, token_length, stdout );
//			printf ("\n");	
			{p++; goto _out; }
		}}
	break;
	case 38:
#line 105 "l.rl"
	{te = p;p--;{EMIT (GREATER); {p++; goto _out; }}}
	break;
	case 39:
#line 107 "l.rl"
	{te = p;p--;{EMIT (LESS); {p++; goto _out; }}}
	break;
	case 40:
#line 114 "l.rl"
	{te = p;p--;{ yylval.str=mkstring(ts, te-ts);  EMIT(IDENTIFIER); {p++; goto _out; } }}
	break;
	case 41:
#line 115 "l.rl"
	{te = p;p--;{ yylval.str=mkstring(ts, te-ts);  EMIT(PACKAGE_NAME); {p++; goto _out; } }}
	break;
	case 42:
#line 1 "NONE"
	{	switch( act ) {
	case 0:
	{{cs = 0; goto _again;}}
	break;
	case 18:
	{{p = ((te))-1;}
		    EMIT(STRING_VALUE); 
//			fwrite( token_start, 1, token_length, stdout );
//			printf ("\n");	
			{p++; goto _out; }
		}
	break;
	case 32:
	{{p = ((te))-1;} yylval.str=mkstring(ts, te-ts);  EMIT(NUMBER); {p++; goto _out; } }
	break;
	case 34:
	{{p = ((te))-1;} yylval.str=mkstring(ts, te-ts);  EMIT(PACKAGE_NAME); {p++; goto _out; } }
	break;
	}
	}
	break;
	case 43:
#line 121 "l.rl"
	{te = p+1;}
	break;
	case 44:
#line 123 "l.rl"
	{te = p+1;{ EMIT (KEEP); cs = 134;	{p++; goto _out; }  }}
	break;
	case 45:
#line 126 "l.rl"
	{te = p+1;{ EMIT (PACKAGE); cs = 141;	{p++; goto _out; }}}
	break;
	case 46:
#line 127 "l.rl"
	{te = p+1;{ EMIT (VERSION); cs = 141;	{p++; goto _out; } }}
	break;
	case 47:
#line 129 "l.rl"
	{te = p+1;{ EMIT (DEPENDS); cs = 141;	{p++; goto _out; }	}}
	break;
	case 48:
#line 130 "l.rl"
	{te = p+1;{EMIT (CONFLICTS); cs = 141; {p++; goto _out; }}}
	break;
	case 49:
#line 131 "l.rl"
	{te = p+1;{EMIT (INSTALLED); cs = 139; {p++; goto _out; }}}
	break;
	case 50:
#line 132 "l.rl"
	{te = p+1;{EMIT (PROVIDES); cs = 141; {p++; goto _out; }}}
	break;
	case 51:
#line 133 "l.rl"
	{te = p+1;{EMIT (INSTALL); cs = 141; {p++; goto _out; }}}
	break;
	case 52:
#line 134 "l.rl"
	{te = p+1;{EMIT (REMOVE); cs = 141; {p++; goto _out; }}}
	break;
	case 53:
#line 135 "l.rl"
	{te = p+1;{EMIT (UPGRADE); cs = 141; {p++; goto _out; }}}
	break;
	case 54:
#line 136 "l.rl"
	{te = p+1;{EMIT (RECOMMENDS); cs = 141; {p++; goto _out; }}}
	break;
	case 55:
#line 150 "l.rl"
	{te = p+1;{ {cs = 137; goto _again;}}}
	break;
	case 56:
#line 119 "l.rl"
	{te = p;p--;}
	break;
	case 57:
#line 120 "l.rl"
	{te = p;p--;}
	break;
	case 58:
#line 125 "l.rl"
	{te = p;p--;{EMIT (PREAMBLE);cs = 141;	{p++; goto _out; }	}}
	break;
	case 59:
#line 128 "l.rl"
	{te = p;p--;{ EMIT (REQUEST); cs = 141;	{p++; goto _out; }  }}
	break;
	case 60:
#line 150 "l.rl"
	{te = p;p--;{ {cs = 137; goto _again;}}}
	break;
#line 907 "Lexer.cc"
		}
	}

_again:
	_acts = _cud_actions + _cud_to_state_actions[cs];
	_nacts = (unsigned int) *_acts++;
	while ( _nacts-- > 0 ) {
		switch ( *_acts++ ) {
	case 2:
#line 1 "NONE"
	{ts = 0;}
	break;
	case 3:
#line 1 "NONE"
	{act = 0;}
	break;
#line 924 "Lexer.cc"
		}
	}

	if ( cs == 0 )
		goto _out;
	if ( ++p != pe )
		goto _resume;
	_test_eof: {}
	if ( p == eof )
	{
	if ( _cud_eof_trans[cs] > 0 ) {
		_trans = _cud_eof_trans[cs] - 1;
		goto _eof_trans;
	}
	}

	_out: {}
	}

#line 211 "l.rl"
        // End of automaton


        /* Check if we failed. */
        if (cs == cud_error) {
            /* Machine failed before finding a token. */
            printf("LEXER ERROR:%d:", current_line);
            fwrite(token_start, 1, token_length, stdout);
            exit(1);
        }

        if (token_read) {
            token_read = false;
            if (stop == pe) {
                if (last_buffer) {
                    done = true;
                } else {
                    should_read = true;
                    have = 0;
                }
            }
            /*printf("read ");
            printf("'");
            fwrite( token_start, 1, token_length, stdout );printf("'");
            printf("\n"); */
            return token;
        } else if (last_buffer) {
            done = true;
        } else {
            // The current buffer was finished without
            //   reading a token at the end
            should_read = true;
            if (ts == 0)
                have = 0;
            else {
                /* There is data that needs to be shifted over. */
                have = pe - ts;
                memmove(buf, ts, have);
                te -= (ts - buf);
                ts = buf;
            }
        }
        assert (!token_read);
        return yylex_internal();
    }
    

Lexer* lexer;
int get_current_line() { return lexer->get_current_line();}
int yylex() {return lexer->yylex_internal ();}
