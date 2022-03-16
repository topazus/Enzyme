// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

/*
 *   File "lstm_b_tapenade_generated.c" is generated by Tapenade 3.14 (r7259) from this file.
 *   To reproduce such a generation you can use Tapenade CLI
 *   (can be downloaded from http://www-sop.inria.fr/tropics/tapenade/downloading.html)
 *
 *   After installing use the next command to generate a file:
 *
 *      tapenade -b -o lstm_tapenade -head "lstm_objective(loss)/(main_params extra_params)" lstm.c
 *
 *   This will produce a file "lstm_tapenade_b.c" which content will be the same as the content of the file "lstm_b_tapenade_generated.c",
 *   except one-line header. Moreover a log-file "lstm_tapenade_b.msg" will be produced.
 *
 *   NOTE: the code in "lstm_b_tapenade_generated.c" is wrong and won't work.
 *         REPAIRED SOURCE IS STORED IN THE FILE "lstm_b.c".
 *         You can either use diff tool or read "lstm_b.c" header to figure out what changes was performed to fix the code.
 *
 *   NOTE: you can also use Tapenade web server (http://tapenade.inria.fr:8080/tapenade/index.jsp)
 *         for generating but the result can be slightly different.
 */

#include "../adbench/lstm.h"

extern "C" {
#include "lstm.h"

// UTILS
// Sigmoid on scalar
double sigmoid(double x)
{
    return 1.0 / (1.0 + exp(-x));
}

// log(sum(exp(x), 2))
double logsumexp(double const* vect, int sz)
{
    double sum = 0.0;
    int i;

    for (i = 0; i < sz; i++)
    {
        sum += exp(vect[i]);
    }

    sum += 2;
    return log(sum);
}

// LSTM OBJECTIVE
// The LSTM model
void lstm_model(
    int hsize,
    double const* __restrict weight,
    double const* __restrict bias,
    double* __restrict hidden,
    double* __restrict cell,
    double const* __restrict input
)
{
    // TODO NOTE THIS
    //__builtin_assume(hsize > 0);

    double* gates = (double*)malloc(4 * hsize * sizeof(double));
    double* forget = &(gates[0]);
    double* ingate = &(gates[hsize]);
    double* outgate = &(gates[2 * hsize]);
    double* change = &(gates[3 * hsize]);

    int i;
    // caching input
    // hidden (needed)
    for (i = 0; i < hsize; i++)
    {
        forget[i] = sigmoid(input[i] * weight[i] + bias[i]);
        ingate[i] = sigmoid(hidden[i] * weight[hsize + i] + bias[hsize + i]);
        outgate[i] = sigmoid(input[i] * weight[2 * hsize + i] + bias[2 * hsize + i]);
        change[i] = tanh(hidden[i] * weight[3 * hsize + i] + bias[3 * hsize + i]);
    }

    // caching cell (needed)
    for (i = 0; i < hsize; i++)
    {
        cell[i] = cell[i] * forget[i] + ingate[i] * change[i];
    }

    for (i = 0; i < hsize; i++)
    {
        hidden[i] = outgate[i] * tanh(cell[i]);
    }

    free(gates);
}

// Predict LSTM output given an input
void lstm_predict(
    int l,
    int b,
    double const* __restrict w,
    double const* __restrict w2,
    double* __restrict s,
    double const* __restrict x,
    double* __restrict x2
)
{
    int i;
    for (i = 0; i < b; i++)
    {
        x2[i] = x[i] * w2[i];
    }

    double* xp = x2;
    for (i = 0; i <= 2 * l * b - 1; i += 2 * b)
    {
        lstm_model(b, &(w[i * 4]), &(w[(i + b) * 4]), &(s[i]), &(s[i + b]), xp);
        xp = &(s[i]);
    }

    for (i = 0; i < b; i++)
    {
        x2[i] = xp[i] * w2[b + i] + w2[2 * b + i];
    }
}

// LSTM objective (loss function)
void lstm_objective(
    int l,
    int c,
    int b,
    double const* __restrict main_params,
    double const* __restrict extra_params,
    double* __restrict state,
    double const* __restrict sequence,
    double* __restrict loss
)
{
    int i, t;
    double total = 0.0;
    int count = 0;
    const double* input = &(sequence[0]);
    double* ypred = (double*)malloc(b * sizeof(double));
    double* ynorm = (double*)malloc(b * sizeof(double));
    const double* ygold;
    double lse;

    __builtin_assume(b>0);
    for (t = 0; t <= (c - 1) * b - 1; t += b)
    {
        lstm_predict(l, b, main_params, extra_params, state, input, ypred);
        lse = logsumexp(ypred, b);
        for (i = 0; i < b; i++)
        {
            ynorm[i] = ypred[i] - lse;
        }

        ygold = &(sequence[t + b]);
        for (i = 0; i < b; i++)
        {
            total += ygold[i] * ynorm[i];
        }

        count += b;
        input = ygold;
    }

    *loss = -total / count;

    free(ypred);
    free(ynorm);
}

extern int enzyme_const;
extern int enzyme_dup;
extern int enzyme_dupnoneed;
void __enzyme_autodiff(...) noexcept;

// *      tapenade -b -o lstm_tapenade -head "lstm_objective(loss)/(main_params extra_params)" lstm.c

void dlstm_objective(
    int l,
    int c,
    int b,
    double const* main_params,
    double* dmain_params,
    double const* extra_params,
    double* dextra_params,
    double* state,
    double const* sequence,
    double* loss,
    double* dloss
)
{
    __enzyme_autodiff(lstm_objective,
        enzyme_const, l,
        enzyme_const, c,
        enzyme_const, b,
        enzyme_dup, main_params, dmain_params,
        enzyme_dup, extra_params, dextra_params,
        enzyme_const, state,
        enzyme_const, sequence,
        enzyme_dupnoneed, loss, dloss
    );
}

}


//! Tapenade
extern "C" {

#include <adBuffer.h>

/*
  Differentiation of sigmoid in reverse (adjoint) mode:
   gradient     of useful results: sigmoid
   with respect to varying inputs: x
*/
// UTILS
// Sigmoid on scalar
void sigmoid_b(double x, double *xb, double sigmoidb) {
    double temp;
    double sigmoid;
    temp = exp(-x) + 1.0;
    *xb = exp(-x)*sigmoidb/(temp*temp);
}

// UTILS
// Sigmoid on scalar
double sigmoid_nodiff(double x) {
    return 1.0/(1.0+exp(-x));
}

/*
  Differentiation of logsumexp in reverse (adjoint) mode:
   gradient     of useful results: logsumexp *vect
   with respect to varying inputs: *vect
   Plus diff mem management of: vect:in
*/
// log(sum(exp(x), 2))
void logsumexp_b(const double *vect, double *vectb, int sz, double logsumexpb)
{
    double sum = 0.0;
    double sumb = 0.0;
    int i;
    double logsumexp;
    for (i = 0; i < sz; ++i)
        sum = sum + exp(vect[i]);
    sum = sum + 2;
    sumb = logsumexpb/sum;
    for (i = sz-1; i > -1; --i)
        vectb[i] = vectb[i] + exp(vect[i])*sumb;
}

// log(sum(exp(x), 2))
double logsumexp_nodiff(const double *vect, int sz) {
    double sum = 0.0;
    int i;
    for (i = 0; i < sz; ++i)
        sum += exp(vect[i]);
    sum += 2;
    return log(sum);
}

/*
  Differentiation of lstm_model in reverse (adjoint) mode:
   gradient     of useful results: alloc(*gates) *cell *bias *hidden
                *weight *input
   with respect to varying inputs: alloc(*gates) *cell *bias *hidden
                *weight *input
   Plus diff mem management of: cell:in bias:in hidden:in weight:in
                input:in
*/
// LSTM OBJECTIVE
// The LSTM model
void lstm_model_b(int hsize, const double *weight, double *weightb, const
        double *bias, double *biasb, double *hidden, double *hiddenb, double *
        cell, double *cellb, const double *input, double *inputb) {
    double *gates;
    double *gatesb;
    double arg1;
    double arg1b;
    int ii1;
    double temp;
    double tempb;
    gatesb = (double *)malloc(4*hsize*sizeof(double));
    for (ii1 = 0; ii1 < 4*hsize; ++ii1)
        gatesb[ii1] = 0.0;
    gates = (double *)malloc(4*hsize*sizeof(double));
    double *forget = &(gates[0]);
    double *forgetb = &(gatesb[0]);
    double *ingate = &(gates[hsize]);
    double *ingateb = &(gatesb[hsize]);
    double *outgate = &(gates[2*hsize]);
    double *outgateb = &(gatesb[2*hsize]);
    double *change = &(gates[3*hsize]);
    double *changeb = &(gatesb[3*hsize]);
    int i;
    for (i = 0; i < hsize; ++i) {
        arg1 = input[i]*weight[i] + bias[i];
        forget[i] = sigmoid_nodiff(arg1);
        arg1 = hidden[i]*weight[hsize+i] + bias[hsize + i];
        ingate[i] = sigmoid_nodiff(arg1);
        arg1 = input[i]*weight[2*hsize+i] + bias[2*hsize + i];
        outgate[i] = sigmoid_nodiff(arg1);
        change[i] = tanh(hidden[i]*weight[3*hsize+i] + bias[3*hsize + i]);
    }
    for (i = 0; i < hsize; ++i) {
        pushReal8(cell[i]);
        cell[i] = cell[i]*forget[i] + ingate[i]*change[i];
    }
    for (i = hsize-1; i > -1; --i) {
        outgateb[i] = outgateb[i] + tanh(cell[i])*hiddenb[i];
        cellb[i] = cellb[i] + outgate[i]*(1.0-tanh(cell[i])*tanh(cell[i]))*
            hiddenb[i];
        hiddenb[i] = 0.0;
    }
    for (i = hsize-1; i > -1; --i) {
        popReal8(&(cell[i]));
        forgetb[i] = forgetb[i] + cell[i]*cellb[i];
        ingateb[i] = ingateb[i] + change[i]*cellb[i];
        changeb[i] = changeb[i] + ingate[i]*cellb[i];
        cellb[i] = forget[i]*cellb[i];
    }
    for (i = hsize-1; i > -1; --i) {
        temp = weight[3*hsize + i];
        tempb = (1.0-tanh(hidden[i]*temp+bias[3*hsize+i])*tanh(hidden[i]*temp+
            bias[3*hsize+i]))*changeb[i];
        hiddenb[i] = hiddenb[i] + temp*tempb;
        weightb[3*hsize + i] = weightb[3*hsize + i] + hidden[i]*tempb;
        biasb[3*hsize + i] = biasb[3*hsize + i] + tempb;
        changeb[i] = 0.0;
        arg1 = input[i]*weight[2*hsize+i] + bias[2*hsize + i];
        sigmoid_b(arg1, &arg1b, outgateb[i]);
        outgateb[i] = 0.0;
        inputb[i] = inputb[i] + weight[2*hsize+i]*arg1b;
        weightb[2*hsize + i] = weightb[2*hsize + i] + input[i]*arg1b;
        biasb[2*hsize + i] = biasb[2*hsize + i] + arg1b;
        arg1 = hidden[i]*weight[hsize+i] + bias[hsize + i];
        sigmoid_b(arg1, &arg1b, ingateb[i]);
        ingateb[i] = 0.0;
        hiddenb[i] = hiddenb[i] + weight[hsize+i]*arg1b;
        weightb[hsize + i] = weightb[hsize + i] + hidden[i]*arg1b;
        biasb[hsize + i] = biasb[hsize + i] + arg1b;
        arg1 = input[i]*weight[i] + bias[i];
        sigmoid_b(arg1, &arg1b, forgetb[i]);
        forgetb[i] = 0.0;
        inputb[i] = inputb[i] + weight[i]*arg1b;
        weightb[i] = weightb[i] + input[i]*arg1b;
        biasb[i] = biasb[i] + arg1b;
    }
    free(gates);
    free(gatesb);
}

// LSTM OBJECTIVE
// The LSTM model
void lstm_model_nodiff(int hsize, const double *weight, const double *bias,
        double *hidden, double *cell, const double *input) {
    double *gates;
    double arg1;
    gates = (double *)malloc(4*hsize*sizeof(double));
    double *forget = &(gates[0]);
    double *ingate = &(gates[hsize]);
    double *outgate = &(gates[2*hsize]);
    double *change = &(gates[3*hsize]);
    int i;
    for (i = 0; i < hsize; ++i) {
        arg1 = input[i]*weight[i] + bias[i];
        forget[i] = sigmoid_nodiff(arg1);
        arg1 = hidden[i]*weight[hsize+i] + bias[hsize + i];
        ingate[i] = sigmoid_nodiff(arg1);
        arg1 = input[i]*weight[2*hsize+i] + bias[2*hsize + i];
        outgate[i] = sigmoid_nodiff(arg1);
        change[i] = tanh(hidden[i]*weight[3*hsize+i] + bias[3*hsize + i]);
    }
    for (i = 0; i < hsize; ++i)
        cell[i] = cell[i]*forget[i] + ingate[i]*change[i];
    for (i = 0; i < hsize; ++i)
        hidden[i] = outgate[i]*tanh(cell[i]);
    free(gates);
}

/*
  Differentiation of lstm_predict in reverse (adjoint) mode:
   gradient     of useful results: alloc(*gates) *s *w *w2 *x2
   with respect to varying inputs: alloc(*gates) *s *w *w2 *x2
   Plus diff mem management of: s:in w:in w2:in x2:in
*/
// Predict LSTM output given an input
void lstm_predict_b(int l, int b, const double *w, double *wb, const double *
        w2, double *w2b, double *s, double *sb, const double *x, double *x2,
        double *x2b) {
    int i;
    double tmp;
    double tmpb;
    for (i = 0; i < b; ++i) {
        pushReal8(x2[i]);
        x2[i] = x[i]*w2[i];
    }
    double *xp = x2;
    double *xpb = x2b;
    for (i = 0; i <= 2*l*b-1; i += 2*b) {
        pushReal8Array(s + i, 2 * b); /* TFIX */
        lstm_model_nodiff(b, &(w[i*4]), &(w[(i+b)*4]), &(s[i]), &(s[i + b]),
                          xp);
        pushPointer8(xpb);
        xpb = &(sb[i]);
        pushPointer8(xp);
        xp = &(s[i]);
    }
    for (i = 0; i < b; ++i) {
        tmp = xp[i]*w2[b+i] + w2[2*b + i];
        pushReal8(x2[i]);
        x2[i] = tmp;
    }
    for (i = b-1; i > -1; --i) {
        popReal8(&(x2[i]));
        tmpb = x2b[i];
        x2b[i] = 0.0;
        xpb[i] = xpb[i] + w2[b+i]*tmpb;
        w2b[b + i] = w2b[b + i] + xp[i]*tmpb;
        w2b[2*b + i] = w2b[2*b + i] + tmpb;
    }
    for (i = 2*l*b-(2*l*b-1)%(2*b)-1; i >= 0; i += -(2*b)) { /* TFIX */
        popPointer8((void **)&xp);
        popPointer8((void **)&xpb);
        popReal8Array(s + i, 2 * b); /* TFIX */
        lstm_model_b(b, &(w[i*4]), &(wb[i*4]), &(w[(i+b)*4]), &(wb[(i+b)*4]),
                     &(s[i]), &(sb[i]), &(s[i + b]), &(sb[i + b]), xp, xpb);
    }
    for (i = b-1; i > -1; --i) {
        popReal8(&(x2[i]));
        w2b[i] = w2b[i] + x[i]*x2b[i];
        x2b[i] = 0.0;
    }
}

// Predict LSTM output given an input
void lstm_predict_nodiff(int l, int b, const double *w, const double *w2,
        double *s, const double *x, double *x2) {
    int i;
    for (i = 0; i < b; ++i)
        x2[i] = x[i]*w2[i];
    double *xp = x2;
    for (i = 0; i <= 2*l*b-1; i += 2*b) {
        lstm_model_nodiff(b, &(w[i*4]), &(w[(i+b)*4]), &(s[i]), &(s[i + b]),
                          xp);
        xp = &(s[i]);
    }
    for (i = 0; i < b; ++i)
        x2[i] = xp[i]*w2[b+i] + w2[2*b + i];
}

/*
  Differentiation of lstm_objective in reverse (adjoint) mode:
   gradient     of useful results: *loss
   with respect to varying inputs: *main_params *extra_params
                *loss
   RW status of diff variables: *main_params:out *extra_params:out
                *loss:in-out
   Plus diff mem management of: extra_params:in loss:in
*/
// LSTM objective (loss function)
void lstm_objective_b(int l, int c, int b, const double *main_params, double *
        main_paramsb, const double *extra_params, double *extra_paramsb,
        double *state, const double *sequence, double *loss, double *lossb) {
    int i, t;
    double total = 0.0;
    double totalb = 0.0;
    int count = 0;
    const double *input = &(sequence[0]);
    double *ypred;
    double *ypredb;
    int ii1;
    int branch;
    double* stateb = (double*)malloc(2 * l * b * sizeof(double)); /* TFIX */
    ypredb = (double *)malloc(b*sizeof(double));
    for (ii1 = 0; ii1 < b; ++ii1)
        ypredb[ii1] = 0.0;
    ypred = (double *)malloc(b*sizeof(double));
    double *ynorm;
    double *ynormb;
    ynormb = (double *)malloc(b*sizeof(double));
    for (ii1 = 0; ii1 < b; ++ii1)
        ynormb[ii1] = 0.0;
    ynorm = (double *)malloc(b*sizeof(double));
    const double* ygold = NULL; /* TFIX */
    double lse;
    double lseb;
    for (t = 0; t <= (c-1)*b-1; t += b) {
        if (ypred) {
            pushReal8Array(ypred, b); /* TFIX */
            pushControl1b(1);
        } else
            pushControl1b(0);
        pushReal8Array(state, 2 * b * l); /* TFIX */
        lstm_predict_nodiff(l, b, main_params, extra_params, state, input,
                            ypred);
        pushPointer8((void*)ygold);
        ygold = &(sequence[t + b]);
        count = count + b;
        pushPointer8((void*)input);
        input = ygold;
    }
    totalb = -(*lossb/count);
    *lossb = 0.0;
    for (ii1 = 0; ii1 < 8 * l * b; ii1++) /* TFIX */
        main_paramsb[ii1] = 0.0;
    for (ii1 = 0; ii1 < 3 * b; ii1++) /* TFIX */
        extra_paramsb[ii1] = 0.0;
    for (t = 0; t < 2 * l * b; t++) /* TFIX */
        stateb[t] = 0.0;
    for (t = (c-1)*b-((c-1)*b-1)%b-1; t >= 0; t += -b) { /* TFIX */
        popPointer8((void **)&input);
        for (i = b-1; i > -1; --i)
            ynormb[i] = ynormb[i] + ygold[i]*totalb;
        popPointer8((void **)&ygold);
        lseb = 0.0;
        for (i = b-1; i > -1; --i) {
            ypredb[i] = ypredb[i] + ynormb[i];
            lseb = lseb - ynormb[i];
            ynormb[i] = 0.0;
        }
        logsumexp_b(ypred, ypredb, b, lseb);
        popReal8Array(state, 2 * b * l); /* TFIX */
        popControl1b(&branch);
        if (branch == 1)
            popReal8Array(ypred, b); /* TFIX */
        lstm_predict_b(l, b, main_params, main_paramsb, extra_params,
                       extra_paramsb, state, stateb, input, ypred, ypredb);
    }
    free(ynorm);
    free(ynormb);
    free(ypred);
    free(ypredb);
    free(stateb); /* TFIX */ // Added to dispose memory allocated in repaired code
}


}


#if 1
//! Adept
#include <adept_source.h>
#include <adept.h>
#include <adept_arrays.h>
using adept::adouble;
using adept::aVector;

namespace adeptTest {
// Sigmoid on scalar
template<typename T>
T sigmoid(T x) {
    return adouble(1) / (adouble(1) + exp(-x));
}

// log(sum(exp(x), 2))
template<typename T>
T logsumexp(const T* vect, int sz) {
    T sum = 0.0;
    for (int i = 0; i < sz; ++i)
        sum += exp(vect[i]);
    sum += adouble(2);
    return log(sum);
}

// LSTM OBJECTIVE

// The LSTM model
template<typename T>
void lstm_model(
    int hsize,
    T* weight,
    T* bias,
    T* hidden,
    T* cell,
    T* input
)
{

    T* gates = new T[4*hsize];
    T* forget = &(gates[0]);
    T* ingate = &(gates[hsize]);
    T* outgate = &(gates[2 * hsize]);
    T* change = &(gates[3 * hsize]);

    int i;
    // caching input
    // hidden (needed)
    for (i = 0; i < hsize; i++)
    {
        forget[i] = sigmoid<adouble>(input[i] * weight[i] + bias[i]);
        ingate[i] = sigmoid<adouble>(hidden[i] * weight[hsize + i] + bias[hsize + i]);
        outgate[i] = sigmoid<adouble>(input[i] * weight[2 * hsize + i] + bias[2 * hsize + i]);
        change[i] = tanh(hidden[i] * weight[3 * hsize + i] + bias[3 * hsize + i]);
    }

    // caching cell (needed)
    for (i = 0; i < hsize; i++)
    {
        cell[i] = cell[i] * forget[i] + ingate[i] * change[i];
    }

    for (i = 0; i < hsize; i++)
    {
        hidden[i] = outgate[i] * tanh(cell[i]);
    }

    delete[] gates;
}

// Predict LSTM output given an input
template<typename T>
void lstm_predict(
    int l,
    int b,
    T* w,
    T* w2,
    T* s,
    T* x,
    T* x2
)
{
    int i;
    for (i = 0; i < b; i++)
    {
        x2[i] = x[i] * w2[i];
    }

    T* xp = x2;
    for (i = 0; i <= 2 * l * b - 1; i += 2 * b)
    {
        lstm_model(b, &(w[i * 4]), &(w[(i + b) * 4]), &(s[i]), &(s[i + b]), xp);
        xp = &(s[i]);
    }

    for (i = 0; i < b; i++)
    {
        x2[i] = xp[i] * w2[b + i] + w2[2 * b + i];
    }
}

// LSTM objective (loss function)
template<typename T>
void lstm_objective(
    int l,
    int c,
    int b,
    T * __restrict main_params,
    T * __restrict extra_params,
    T* __restrict state,
    T * __restrict sequence,
    T* __restrict loss
)
{
    int i, t;
    T total = 0.0;
    int count = 0;
    T* input = &(sequence[0]);
    T* ypred = new T[b];
    T* ynorm = new T[b];
    T* ygold;
    T lse;

    __builtin_assume(b>0);
    for (t = 0; t <= (c - 1) * b - 1; t += b)
    {
        lstm_predict(l, b, main_params, extra_params, state, input, ypred);
        lse = logsumexp(ypred, b);
        for (i = 0; i < b; i++)
        {
            ynorm[i] = ypred[i] - lse;
        }

        ygold = &(sequence[t + b]);
        for (i = 0; i < b; i++)
        {
            total += ygold[i] * ynorm[i];
        }

        count += b;
        input = ygold;
    }

    *loss = -total / adouble(count);

    delete[] ypred;
    delete[] ynorm;
}
};

// Note ADBench did not have an adept impl
void adept_dlstm_objective(int l, int c, int b, const double *main_params, double *
        main_paramsb, const double *extra_params, double *extra_paramsb,
        double *state, const double *sequence, double *loss, double *lossb) {

    int main_sz = 2 * l * 4 * b;
    int extra_sz = 3 * b;
    int state_sz = 2 * l * b;
    int seq_sz = c* b;

  adept::Stack stack;

  adouble *amain = new adouble[main_sz];
  adouble *aextra = new adouble[extra_sz];
  adouble *astate = new adouble[state_sz];
  adouble *aseq = new adouble[seq_sz];

      adept::set_values(amain, main_sz, main_params);
      adept::set_values(aextra, extra_sz, extra_params);
      adept::set_values(astate, state_sz, state);
      adept::set_values(aseq, seq_sz, sequence);

      adouble aloss;

      stack.new_recording();
      adouble aerr;

      adeptTest::lstm_objective(l, c, b, amain, aextra, astate, aseq, &aloss);
      aloss.set_gradient(1.); // only one J row here

      stack.compute_adjoint();

      adept::get_gradients(amain, main_sz, main_paramsb);
      adept::get_gradients(aextra, extra_sz, extra_paramsb);

}
#endif