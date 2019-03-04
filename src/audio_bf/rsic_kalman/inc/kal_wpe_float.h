
#define FRAME_LEN  512
#define NRE 257
#define WPE_L  16
#define KAL_ALPHA  0.99f

typedef float f_t;
typedef struct _z_t{f_t re, im;} z_t;

static inline z_t z_add(z_t a, z_t b){
    return (z_t){.re=a.re+b.re, .im=a.im+b.im};
}
static inline z_t z_sub(z_t a, z_t b){
    return (z_t){.re=a.re-b.re, .im=a.im-b.im};
}
static inline z_t z_conj(z_t a){
    return (z_t){.re=a.re, .im=-a.im};
}
static inline z_t z_scale(z_t a, float b){
    return (z_t){.re=a.re*b, .im=a.im*b};
}
static inline z_t z_mul(z_t a, z_t b){
    return (z_t){.re=(a.re*b.re - a.im*b.im), .im=(a.re*b.im + a.im*b.re)};
}
static inline float z_eng(z_t a){
    return (a.re*a.re) + (a.im*a.im);
}
static inline z_t z_inv(z_t a){
    float denm = (a.re*a.re) + (a.im*a.im);
    return (z_t){.re=a.re/denm, .im=-a.im/denm};
}
static inline z_t z_mat_mul(z_t* a, z_t* b, z_t* ret, int rol, int mid, int col){
    for(int i=0; i<rol; i++){
        for(int j=0; j<col; j++){
            z_t acc = (z_t){.re=0, .im=0};
            for(int t=0; t<mid; t++){
                acc = z_add(acc, z_mul(a[i*mid + t], b[t*col + j]));
            }
            ret[i*col + j] = acc;
        }
    }
}
static inline z_t z_vec_dot(z_t* a, z_t* b, int len){
    z_t acc = (z_t){.re=0, .im=0};
    for(int i=0; i<len; i++){
        acc = z_add(acc, z_mul(a[i], b[i]));
    }
    return acc;
}
static inline z_t z_vec_dot_conj(z_t* a, z_t* conj_b, int len){
    z_t acc = (z_t){.re=0, .im=0};
    for(int i=0; i<len; i++){
        acc = z_add(acc, z_mul(a[i], z_conj(conj_b[i])));
    }
    return acc;
}
static inline void z_vec_scale_real(z_t* a, float s, int len){
    for(int i=0; i<len; i++){
        a[i] = (z_t){.re=a[i].re*s, .im=a[i].im*s};
    }
}

void kalman_wpe_float_init();
void kalman_wpe_float(z_t data[NRE]);