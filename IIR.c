/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2018 UniZG, Zagreb. All rights reserved.
 * (c) 2018 FER, Zagreb. All rights reserved.
 */

/*!
  \file   IIR.cpp
  \brief

  <long description>

  \author Marin Parmać
  \date   2018-03-20
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <math.h>



/*** DECLARATIONS ***/

enum filterType_enum;
struct IIR;

int16_t  IIRFilter_direct2(int16_t input,struct IIR * iir);
struct IIR * createIIR(unsigned int const N);
int16_t readFirstWord(FILE *fp);
struct IIR * getCoeffsAndInit_direct(char file_name[50]);
void writeState(int16_t input, struct IIR * iir);
int16_t readState(struct IIR * iir);
void decrementStateIndex(struct IIR * iir);
void incrementStateIndex(struct IIR * iir);
void deleteIIR(struct IIR * iir);

/*** STRUCTURES ***/

enum filterType_enum { 											// Ako ce se spajati IIR filtri.
	IIR_Direct,													// ptr->filterType = 0;
	IIR_Direct_transposed,										// ptr->filterType = 1;
	IIR_Cascade,												// ptr->filterType = 2;
	IIR_Parallel												// ptr->filterType = 3;
};

struct IIR{
	unsigned int L;
	int16_t *coeffs;
	int16_t *state;
	enum filterType_enum filterType;
	unsigned int stateIndex;
};

/*** FUCTIONS ***/

struct IIR * createIIR(unsigned int const N)					// Ako ce se spajati IIR filti dodati jos da prima fylterType
{

	struct IIR *ptr = NULL;

	ptr = (struct IIR *) malloc (sizeof( struct IIR ));
	assert (NULL != ptr);
		if (NULL == ptr) return ptr;

	ptr->L = 0;
	ptr->coeffs = NULL;
	ptr->state = NULL;
	ptr->stateIndex = 0;
	ptr->filterType = 0;										// 0 -> direktna -- Treba doraditi fju ako ce se spajati iir filtri. Npr. ovdje treba staviti ptr->filterType = fylterType (ulazna vrijednost)!!!


	assert(NULL == ptr->coeffs);
	ptr->coeffs = (int16_t *) malloc ((2*N+3) * sizeof(int16_t));
	assert(NULL != ptr->coeffs);

	assert(NULL == ptr->state);
	ptr->state = (int16_t *) malloc (N * sizeof(int16_t));
	assert(NULL != ptr->state);

	if ((NULL == ptr->coeffs) || (NULL == ptr->state)) {
		deleteIIR (ptr);
		return NULL;
	};

	memset (ptr->coeffs, 0, N * sizeof(int16_t));
	memset (ptr->state, 0, N * sizeof(int16_t));

	ptr->L = N;

	return ptr;
};

void deleteIIR(struct IIR * iir)
{

		assert(NULL != iir);
			if (NULL == iir) return;

		if (NULL != iir->coeffs) {
			free (iir->coeffs);
			iir->coeffs = NULL;
				};

		if (NULL != iir->state) {
			free (iir->state);
			iir->state = NULL;
		};

		memset (iir->coeffs, 0, 0);
		memset (iir->state, 0, 0);

		free (iir);

}

int16_t readFirstWord(FILE *fp)
{
	assert(NULL != fp);
	char word[30];
	int16_t hex_word;

	fscanf(fp,"%s%*[^\n]",word);								// Uzme prvi string u nizu.
	hex_word = (int16_t)strtol(word, NULL, 16);     			// String to integer.
	printf("word read is: %hX\n", hex_word);

	return hex_word;
}

struct IIR * getCoeffsAndInit_direct(char file_name[50]) 		// Prima naziv strukture.
{																// Ucitava koeficijente i inicijalizira filtar.

	struct IIR * iir = NULL;

	/* DIREKTNA
	 n
	 amp_in
	a-ovi x (n+1) idu  od a_n do a_0
	b-ovi x (n+1) idu  od b_n do b_0
	amp_out
	p
	*/

	FILE *fp;
	int i = 0;

	fp = fopen(file_name, "r");									 // Otvori datoteku.


	if (fp == NULL) {
		printf("Nije učitana datoteka s koeficijentima!!!\n");
		return iir;
		};

	int16_t N = readFirstWord(fp);								// Ucitaj red filtra

	iir = createIIR(N);											// Inicijalizacija filtra s dohvacenim redom N.


	for (i = 1; i <= (2*N + 5); i++)							// Ucitaj preostale koeficijente
	{

		iir->coeffs[i] = readFirstWord(fp);						// coeffs[1] = amp_in, coeffs[2] = a[N] ... coeffs[2N+4]=amp_out coeffs[2N+5]=p
	}
	fclose (fp);

	return iir;
}

void incrementStateIndex(struct IIR * iir) {

	assert(NULL != iir);
	if (NULL == iir) return;

	int stateIndex = ((iir->stateIndex + 1) % (iir->L));		// Cirkularni spremnik velicine reda N.
	assert((0 <= stateIndex) && (stateIndex < iir->L));

	iir->stateIndex = stateIndex;
}

void decrementStateIndex(struct IIR * iir) {
	int i;
	assert(NULL != iir);
	if (NULL == iir) return;

	if ((iir->stateIndex) == 0) {
			i = ((iir->L)-1);
		} else {
			i = ((iir->stateIndex) - 1);
		};

	assert((0 <= i) && (i < iir->L));

	iir->stateIndex = i;
}

int16_t readState(struct IIR * iir) {

	assert(NULL != iir);
	if (NULL == iir) return 0;

	int i = iir->stateIndex;
	assert((0 <= i) && (i < iir->L));

	return (iir->state[i]);
}

void writeState(int16_t input, struct IIR * iir) {
	unsigned int i = iir->stateIndex;

	assert(NULL != iir);
		if (NULL == iir) return;

	iir->state[i] = input;
}


int16_t  IIRFilter_direct2(int16_t input,struct IIR * iir)
{

	assert ( (NULL != iir) || (iir->filterType == IIR_Direct) );
	if (NULL == iir) return 0;

	int N = iir->L;												// Red filtra.
	int16_t *coeffs_a = &(iir->coeffs[2]);
	int16_t *coeffs_b = &(iir->coeffs[N+3]);
	assert(NULL != coeffs_a || NULL != coeffs_b);
	//printf("\nampin = %d", iir->coeffs[1]);
	//printf("input %d",input);
	int32_t acc_A = ( (int32_t)(input) * (int32_t)(iir->coeffs[1]) ) >> 15;
	int32_t acc_B = 0;
	//printf("\nacc_a in= %d",acc_A);

	decrementStateIndex(iir); 									// Postavi na inicijalno stanje. Na pocetku na zadnje.
	for (int i = 1; i <= N; i++)
	{


		//printf("\n a[%d] = %d",5-i,*coeffs_a);
		//printf("\n b[%d] = %hX",5-i,*coeffs_b);

		int32_t const tmp = readState(iir);						// Procitaj sljedece stanje na koje si se prethodno pozicionirao.

		//printf("\n stanje[%d] = %hX",iir->stateIndex,tmp);
		//printf("\nacc_a prije x2.8 = %d", acc_A);

		acc_A -= ( tmp * (int32_t)(*coeffs_a) ) >> 15;			// ulaz*k_ul - stanje[N]*a[N] - stanje[N-1]*a[N-1]-...-stanje[1]*a[1]
		acc_B += ( tmp * (int32_t)(*coeffs_b) ) >> 15;			// 0 + stanje[N]*b[N] +....+ stanje[1]*b[1]

		//printf("\nacc_a = %d", acc_A);
		//printf("\nacc_b = %d", acc_B);

		decrementStateIndex(iir);								// Pozicioniraj se na sljedece stanje.
		coeffs_a++;coeffs_b++;									// Pozcioniraj se na sljedeci koeficijent. aN -> a(N-1)...->a1..Na kraju se pozicionira na 1/a0 i b0.
	};
	//printf("\n\n\n");

    acc_A = acc_A << *coeffs_a;

   // printf("\nstanje sljedece %d", acc_A);

	writeState((int16_t)(acc_A), iir);							// Zapisi na inicijalno stanje novu vrijednost stanja.(Na pocetku je to zadnje stanje).

	//printf("\nupisano stanje[%d] = %d", iir->stateIndex, (int32_t)iir->state[iir->stateIndex]);

	acc_B += ( acc_A * (int32_t)(*coeffs_b) ) >> 15;

	acc_B = ( acc_B * (int32_t)(iir->coeffs[2*N+4]) ) >> (15 - iir->coeffs[2*N+5]);						/


return (int16_t)(acc_B );										// Koncacni izlaz
}


int main(void) {
	  FILE *input;
	  FILE *output;
	  struct IIR *iir;
	  int i;
	  int16_t signal, y;

	  input = fopen("ULAZ_data.txt", "r");
	  if (NULL == input) {
	    printf("Nije uspijelo otvaranje datoteke s ulaznim podacima !");
	    return EXIT_FAILURE;
	  }else{ puts("otovrilo ulaz");
	  }
	  output = fopen("IZLAZ_data7.txt", "w");
	  if (NULL == output) {
	    printf("Nije uspijelo otvaranje datoteke s izlaznim podacima !");
	    return EXIT_FAILURE;
	  };

	  fseek(input, 0, SEEK_SET);

	  iir = getCoeffsAndInit_direct("words.txt"); //coeffs

	  for (i = 0; i < (10000); i++) {
	    fscanf(input, "%hX", &signal);
	    y = IIRFilter_direct2(signal, iir);

	    printf("Izlaz[%d] = %04hX\n",i,y);

	    fprintf(output, "%04hX\n", y);

	  };
     //coeffs


	 // deleteIIR(iir);
	  fclose(input);
	  fclose(output);

	  system("pause");
}
