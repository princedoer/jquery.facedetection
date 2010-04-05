#include "ccv.h"
#ifdef HAVE_CBLAS
#include <cblas.h>
#endif

double ccv_trace(ccv_matrix_t* mat)
{
	return 0;
}

double ccv_norm(ccv_matrix_t* mat, int type)
{
	return 0;
}

double ccv_sum(ccv_matrix_t* mat)
{
	ccv_dense_matrix_t* dmt = ccv_get_dense_matrix(mat);
	double sum = 0;
	unsigned char* m_ptr = dmt->data.ptr;
	int i, j;
#define for_block(dummy, __for_get) \
	for (i = 0; i < dmt->rows; i++) \
	{ \
		for (j = 0; j < dmt->cols; j++) \
			sum += __for_get(m_ptr, j); \
		m_ptr += dmt->step; \
	}
	ccv_matrix_getter(dmt->type, for_block);
#undef for_block
	return sum;
}

void ccv_gemm(ccv_matrix_t* a, ccv_matrix_t* b, double alpha, ccv_matrix_t* c, double beta, int transpose, ccv_matrix_t** d)
{
	ccv_dense_matrix_t* da = ccv_get_dense_matrix(a);
	ccv_dense_matrix_t* db = ccv_get_dense_matrix(b);
	ccv_dense_matrix_t* dc = (c == NULL) ? NULL : ccv_get_dense_matrix(c);
	ccv_dense_matrix_t* dd;

	assert(da->type == db->type && ((transpose & CCV_A_TRANSPOSE) ? da->rows : da->cols) == ((transpose & CCV_B_TRANSPOSE) ? db->cols : db->rows));

	if (dc != NULL)
		assert(dc->type == da->type && ((transpose & CCV_A_TRANSPOSE) ? da->cols : da->rows) == dc->rows && ((transpose & CCV_B_TRANSPOSE) ? db->rows : db->cols) == dc->cols);
	
	if (*d == NULL)
	{
		if (CCV_IS_EMPTY_SIGNATURE(da) || CCV_IS_EMPTY_SIGNATURE(db) || (dc != NULL && CCV_IS_EMPTY_SIGNATURE(dc)))
		{
			*d = dd = ccv_dense_matrix_new((transpose & CCV_A_TRANSPOSE) ? da->cols : da->rows, (transpose & CCV_B_TRANSPOSE) ? db->rows : db->cols, da->type, NULL, NULL);
		} else {
			int sig[5];
			char identifier[20];
			memset(identifier, 0, 20);
			sprintf(identifier, "ccv_gemm(%d)", transpose);
			if (dc == NULL)
				ccv_matrix_generate_signature(identifier, 20, sig, da->sig, db->sig, NULL);
			else
				ccv_matrix_generate_signature(identifier, 20, sig, da->sig, db->sig, dc->sig, NULL);

			*d = dd = ccv_dense_matrix_new((transpose & CCV_A_TRANSPOSE) ? da->cols : da->rows, (transpose & CCV_B_TRANSPOSE) ? db->rows : db->cols, da->type, NULL, sig);
			if (dd->type & CCV_GARBAGE)
			{
				dd->type &= ~CCV_GARBAGE;
				return;
			}
		}
	} else {
		dd = ccv_get_dense_matrix(*d);
	
		assert(da->type == dd->type && ((transpose & CCV_A_TRANSPOSE) ? da->cols : da->rows) == dd->rows && ((transpose & CCV_B_TRANSPOSE) ? db->rows : db->cols) == dd->cols);
		
		if (dd != dc)
			memcpy(dd->data.ptr, dc->data.ptr, dc->step * dc->rows);
	}
#ifdef HAVE_CBLAS
	switch (CCV_GET_DATA_TYPE(dd->type))
	{
		case CCV_32F:
			cblas_sgemm(CblasRowMajor, (transpose & CCV_A_TRANSPOSE) ? CblasTrans : CblasNoTrans, (transpose & CCV_B_TRANSPOSE) ? CblasTrans : CblasNoTrans, dd->rows, dd->cols, da->cols, alpha, da->data.fl, da->cols, db->data.fl, db->cols, beta, dd->data.fl, dd->cols);
			break;
		case CCV_64F:
			cblas_dgemm(CblasRowMajor, (transpose & CCV_A_TRANSPOSE) ? CblasTrans : CblasNoTrans, (transpose & CCV_B_TRANSPOSE) ? CblasTrans : CblasNoTrans, dd->rows, dd->cols, (transpose & CCV_A_TRANSPOSE) ? da->rows : da->cols, alpha, da->data.db, da->cols, db->data.db, db->cols, beta, dd->data.db, dd->cols);
			break;
	}
#endif
}
