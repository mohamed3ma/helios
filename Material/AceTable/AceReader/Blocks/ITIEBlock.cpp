/*
 Copyright (c) 2012, Esteban Pellegrino
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the <organization> nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ITIEBlock.hpp"
#include "../AceUtils.hpp"
#include "../SabTable.hpp"

using namespace std;

namespace Ace {

ITIEBlock::ITIEBlock(const int nxs[nxs_size], const int jxs[jxs_size], const vector<double>& xss, AceTable* ace_table)
	: ACEBlock(xss,ace_table) {
	/* Begin of ITIE block */
	setBegin(xss.begin() + (jxs[SabTable::ITIE] - 1));

	/* Length */
	int table_length;
	getXSS(table_length);
	getXSS(energy,table_length);
	getXSS(sigma_in,table_length);
}

void ITIEBlock::dump(ostream& xss) {
	int length = energy.size();
	putXSS(length, xss);
	putXSS(energy,xss);
	putXSS(sigma_in,xss);
}

void ITIEBlock::updateData() {}

void ITIEBlock::updatePointers(int nxs[nxs_size], const int jxs_old[jxs_size], int jxs_new[jxs_size]) const {
	/* Recalculate pointers on the JXS array */
	shiftJXSArray(jxs_old,jxs_new,SabTable::ITIE,getSize());
}

int ITIEBlock::getType() const {
	return SabTable::ITIE;
};

int ITIEBlock::getSize() const {
	return energy.size() * 2 + 1;
};


ITIEBlock::~ITIEBlock() {/* */}

} /* namespace Ace */