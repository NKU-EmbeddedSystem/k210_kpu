#include <region_layer.h>
#include <defs.h>
// #include <qsort>
#define LOG(fmt, args...) cprintf(fmt, ##args)

extern void qsort(void *aa, size_t n, size_t es, int (*cmp)(const void *, const void *));
typedef struct box_t
{
    float x, y, w, h;
} box;

typedef struct
{
    int index;
    int classes;
    float **probs;
} sortable_bbox;

#define region_layer_img_w 320
#define region_layer_img_h 240
#define region_layer_net_w 320
#define region_layer_net_h 240
#define region_layer_thresh 0.5
#define region_layer_nms 0.2

// params parse_net_options
#define region_layer_l_h 7  // From 14 conv
#define region_layer_l_w 10 // From 14 conv

// parse_region
#define region_layer_l_classes CLASS_NUMBER
#define region_layer_l_coords 4
#define region_layer_l_n 5
#define region_layer_boxes (region_layer_l_h * region_layer_l_w * region_layer_l_n)                      // l.w * l.h * l.n
#define region_layer_outputs (region_layer_boxes * (region_layer_l_classes + region_layer_l_coords + 1)) // l.h * l.w * l.n * (l.classes + l.coords + 1)

static const float l_biases[] = {1.08, 1.19, 3.42, 4.41, 6.63, 11.38, 9.42, 5.11, 16.62, 10.52};

#ifndef DEBUG_FLOAT
float scale = 0.12349300010531557;
float bais = -13.528213;
static const float activate_array_acc[] = {
    1.3328189694961807e-06,
    1.5080072089742540e-06,
    1.7062224818216272e-06,
    1.9304914885691952e-06,
    2.1842387597655508e-06,
    2.4713389459562257e-06,
    2.7961759803108938e-06,
    3.1637100171258366e-06,
    3.5795531681155441e-06,
    4.0500551926836789e-06,
    4.5824004502774000e-06,
    5.1847175947934816e-06,
    5.8662036854391201e-06,
    6.6372646084138191e-06,
    7.5096739526212477e-06,
    8.4967527641255691e-06,
    9.6135729225188708e-06,
    1.0877187242602649e-05,
    1.2306889812282029e-05,
    1.3924510538508741e-05,
    1.5754748394483340e-05,
    1.7825548451035793e-05,
    2.0168528442077915e-05,
    2.2819461368345285e-05,
    2.5818821496712838e-05,
    2.9212402077042794e-05,
    3.3052014189329888e-05,
    3.7396277367239174e-05,
    4.2311514038479549e-05,
    4.7872761398681535e-05,
    5.4164916117065141e-05,
    6.1284029285689262e-05,
    6.9338771299301614e-05,
    7.8452088923350698e-05,
    8.8763079711325489e-05,
    1.0042911221171431e-04,
    1.1362822410713335e-04,
    1.2856183460701150e-04,
    1.4545781213044456e-04,
    1.6457394363435520e-04,
    1.8620185793861597e-04,
    2.1067146215753940e-04,
    2.3835595795864255e-04,
    2.6967751293744617e-04,
    3.0511367203467946e-04,
    3.4520460475452537e-04,
    3.9056129610640685e-04,
    4.4187480283762508e-04,
    4.9992671181188150e-04,
    5.6560095449340834e-04,
    6.3989715060361525e-04,
    7.2394567532196548e-04,
    8.1902466810731341e-04,
    9.2657922752505161e-04,
    1.0482430655806845e-03,
    1.1858629271716029e-03,
    1.3415261155418714e-03,
    1.5175915031861280e-03,
    1.7167244495632473e-03,
    1.9419360922238340e-03,
    2.1966275263765726e-03,
    2.4846394391905237e-03,
    2.8103078186899476e-03,
    3.1785264120643758e-03,
    3.5948166632909628e-03,
    4.0654059133052606e-03,
    4.5973146950135119e-03,
    5.1984539967680019e-03,
    5.8777333969401357e-03,
    6.6451809829067288e-03,
    7.5120759523160966e-03,
    8.4910947429398254e-03,
    9.5964714371001205e-03,
    1.0844173021768325e-02,
    1.2252089836376122e-02,
    1.3840241183260141e-02,
    1.5630995581686655e-02,
    1.7649304481510950e-02,
    1.9922947377191937e-02,
    2.2482785132417658e-02,
    2.5363016891101636e-02,
    2.8601434160928141e-02,
    3.2239663461266138e-02,
    3.6323386285491546e-02,
    4.0902522011338815e-02,
    4.6031355802030889e-02,
    5.1768589519288997e-02,
    5.8177289324925344e-02,
    6.5324699178315362e-02,
    7.3281885159208845e-02,
    8.2123171924593577e-02,
    9.1925330285711121e-02,
    1.0276647469865213e-01,
    1.1472463241461194e-01,
    1.2787595328893125e-01,
    1.4229254199875552e-01,
    1.5803991374073198e-01,
    1.7517410107690731e-01,
    1.9373847348624904e-01,
    2.1376037132852968e-01,
    2.3524769990866273e-01,
    2.5818567308769047e-01,
    2.8253393374811592e-01,
    3.0822430347358692e-01,
    3.3515941873408950e-01,
    3.6321248925990063e-01,
    3.9222836221175744e-01,
    4.2202599334535196e-01,
    4.5240231892888394e-01,
    4.8313740034843805e-01,
    5.1400059204296478e-01,
    5.4475737951923020e-01,
    5.7517646337852657e-01,
    6.0503663876338565e-01,
    6.3413304170844587e-01,
    6.6228240096865842e-01,
    6.8932703533460848e-01,
    7.1513745683938179e-01,
    7.3961356271418754e-01,
    7.6268450824955780e-01,
    7.8430743775816569e-01,
    8.0446530580483777e-01,
    8.2316404519046549e-01,
    8.4042933546383891e-01,
    8.5630320224535195e-01,
    8.7084064066894806e-01,
    8.8410641277580093e-01,
    8.9617212453610340e-01,
    9.0711364754443402e-01,
    9.1700891590155442e-01,
    9.2593610150939509e-01,
    9.3397215103099540e-01,
    9.4119165444826880e-01,
    9.4766600743724538e-01,
    9.5346282647890634e-01,
    9.5864557556989349e-01,
    9.6327336556031007e-01,
    9.6740089067596846e-01,
    9.7107847102141809e-01,
    9.7435217432710952e-01,
    9.7726399456996682e-01,
    9.7985206915396750e-01,
    9.8215091997082304e-01,
    9.8419170682184731e-01,
    9.8600248436709070e-01,
    9.8760845600206615e-01,
    9.8903221988750412e-01,
    9.9029400382285548e-01,
    9.9141188681082659e-01,
    9.9240200605728868e-01,
    9.9327874883356926e-01,
    9.9405492913645577e-01,
    9.9474194944976591e-01,
    9.9534994816934830e-01,
    9.9588793342518900e-01,
    9.9636390413989406e-01,
    9.9678495921832200e-01,
    9.9715739578148144e-01,
    9.9748679734920165e-01,
    9.9777811284848272e-01,
    9.9803572728404766e-01,
    9.9826352485909964e-01,
    9.9846494528124885e-01,
    9.9864303393355325e-01,
    9.9880048653558950e-01,
    9.9893968886573792e-01,
    9.9906275206434814e-01,
    9.9917154398875263e-01,
    9.9926771704554895e-01,
    9.9935273288334459e-01,
    9.9942788429028695e-01,
    9.9949431460512705e-01,
    9.9955303491817049e-01,
    9.9960493930908023e-01,
    9.9965081834193348e-01,
    9.9969137101400607e-01,
    9.9972721533322595e-01,
    9.9975889767994464e-01,
    9.9978690109138979e-01,
    9.9981165259171079e-01,
    9.9983352967674477e-01,
    9.9985286605032797e-01,
    9.9986995669803413e-01,
    9.9988506237447716e-01,
    9.9989841357164910e-01,
    9.9991021402807356e-01,
    9.9992064383171930e-01,
    9.9992986216355684e-01,
    9.9993800972326030e-01,
    9.9994521087379051e-01,
    9.9995157553736802e-01,
    9.9995720087160378e-01,
    9.9996217275123289e-01,
    9.9996656707796583e-01,
    9.9997045093836956e-01,
    9.9997388362739026e-01,
    9.9997691755309093e-01,
    9.9997959903637923e-01,
    9.9998196901790171e-01,
    9.9998406368287485e-01,
    9.9998591501337397e-01,
    9.9998755127649441e-01,
    9.9998899745583170e-01,
    9.9999027563285658e-01,
    9.9999140532400022e-01,
    9.9999240377859488e-01,
    9.9999328624221029e-01,
    9.9999406618940734e-01,
    9.9999475552945749e-01,
    9.9999536478816764e-01,
    9.9999590326858834e-01,
    9.9999637919305162e-01,
    9.9999679982871337e-01,
    9.9999717159851120e-01,
    9.9999750017923417e-01,
    9.9999779058820082e-01,
    9.9999804725986730e-01,
    9.9999827411353848e-01,
    9.9999847461321156e-01,
    9.9999865182046921e-01,
    9.9999880844122713e-01,
    9.9999894686705204e-01,
    9.9999906921168025e-01,
    9.9999917734329213e-01,
    9.9999927291303936e-01,
    9.9999935738025658e-01,
    9.9999943203474562e-01,
    9.9999949801646970e-01,
    9.9999955633296078e-01,
    9.9999960787470432e-01,
    9.9999965342873587e-01,
    9.9999969369066000e-01,
    9.9999972927527181e-01,
    9.9999976072594388e-01,
    9.9999978852292493e-01,
    9.9999981309067143e-01,
    9.9999983480433063e-01,
    9.9999985399546754e-01,
    9.9999987095712894e-01,
    9.9999988594831779e-01,
    9.9999989919794818e-01,
    9.9999991090834028e-01,
    9.9999992125831128e-01,
    9.9999993040590374e-01,
    9.9999993849080049e-01,
    9.9999994563645733e-01,
    9.9999995195198788e-01,
    9.9999995753382964e-01,
    9.9999996246721679e-01,
    9.9999996682748149e-01,
    9.9999997068120483e-01,
    9.9999997408723296e-01,
    9.9999997709757527e-01,
    9.9999997975819976e-01,
    9.9999998210973351e-01,
    9.9999998418808467e-01,
};

static const float softmax_acc[] = {
    2.1074449194523702e-14,
    2.3844518727161522e-14,
    2.6978691973487115e-14,
    3.0524827484615878e-14,
    3.4537074439384916e-14,
    3.9076699497573622e-14,
    4.4213022336435936e-14,
    5.0024474156103210e-14,
    5.6599795317145092e-14,
    6.4039390398107005e-14,
    7.2456861364636330e-14,
    8.1980742261552172e-14,
    9.2756461916459094e-14,
    1.0494856462521368e-13,
    1.1874322272892007e-13,
    1.3435107944927940e-13,
    1.5201046539214766e-13,
    1.7199103783502335e-13,
    1.9459789837007226e-13,
    2.2017625177872880e-13,
    2.4911667727849253e-13,
    2.8186109263338771e-13,
    3.1890950219953976e-13,
    3.6082763194791878e-13,
    4.0825556805039648e-13,
    4.6191753093956433e-13,
    5.2263293409133495e-13,
    5.9132889639699556e-13,
    6.6905439918751747e-13,
    7.5699630408666410e-13,
    8.5649747628407854e-13,
    9.6907728996918804e-13,
    1.0964548290420683e-12,
    1.2405751373741304e-12,
    1.4036389194577518e-12,
    1.5881361449712457e-12,
    1.7968840703978751e-12,
    2.0330702582857637e-12,
    2.3003012510489390e-12,
    2.6026576425543130e-12,
    2.9447563884328109e-12,
    3.3318213065877413e-12,
    3.7697628444368452e-12,
    4.2652683309270034e-12,
    4.8259040914619187e-12,
    5.4602309850286091e-12,
    6.1779351277648078e-12,
    6.9899757991044506e-12,
    7.9087527890153910e-12,
    8.9482957417066263e-12,
    1.0124478387067348e-11,
    1.1455260931132753e-11,
    1.2960964306858170e-11,
    1.4664580472986235e-11,
    1.6592123499244389e-11,
    1.8773026798912318e-11,
    2.1240592574466494e-11,
    2.4032500339296422e-11,
    2.7191382280576011e-11,
    3.0765474249029650e-11,
    3.4809352316150784e-11,
    3.9384766146035573e-11,
    4.4561581907348645e-11,
    5.0418849123602503e-11,
    5.7046007752462462e-11,
    6.4544253926069910e-11,
    7.3028085207122101e-11,
    8.2627048956632600e-11,
    9.3487720510792359e-11,
    1.0577594137352350e-10,
    1.1967935160172632e-10,
    1.3541025505252439e-10,
    1.5320886124457594e-10,
    1.7334695333639822e-10,
    1.9613203823140579e-10,
    2.2191204218141029e-10,
    2.5108062359002512e-10,
    2.8408318414203171e-10,
    3.2142367004811505e-10,
    3.6367226725939153e-10,
    4.1147410815696367e-10,
    4.6555912266691084e-10,
    5.2675318422636174e-10,
    5.9599072079859214e-10,
    6.7432898350621657e-10,
    7.6296419076328667e-10,
    8.6324979442576854e-10,
    9.7671714688289170e-10,
    1.1050988846740922e-09,
    1.2503553856972936e-09,
    1.4147047040078199e-09,
    1.6006564393096334e-09,
    1.8110500583231479e-09,
    2.0490982531934802e-09,
    2.3184359990183054e-09,
    2.6231760595993607e-09,
    2.9679717889857090e-09,
    3.3580881877827067e-09,
    3.7994822992504136e-09,
    4.2988941728326428e-09,
    4.8639497841220112e-09,
    5.5032774828396523e-09,
    6.2266397469802343e-09,
    7.0450822549962529e-09,
    7.9711025523411638e-09,
    9.0188408878944154e-09,
    1.0204296134324164e-08,
    1.1545570089472246e-08,
    1.3063143889222716e-08,
    1.4780190752654094e-08,
    1.6722929835065921e-08,
    1.8921026592185198e-08,
    2.1408045769078414e-08,
    2.4221963930870834e-08,
    2.7405749361571197e-08,
    3.1008018186007477e-08,
    3.5083776733797136e-08,
    3.9695261481185732e-08,
    4.4912889396590554e-08,
    5.0816333201544687e-08,
    5.7495737966180195e-08,
    6.5053097616558844e-08,
    7.3603812372993640e-08,
    8.3278450901311974e-08,
    9.4224744084952769e-08,
    1.0660984086262589e-07,
    1.2062286057797270e-07,
    1.3647778081538953e-07,
    1.5441670481901083e-07,
    1.7471355838805377e-07,
    1.9767827269979272e-07,
    2.2366151693150794e-07,
    2.5306005294812336e-07,
    2.8632279382115594e-07,
    3.2395765869201747e-07,
    3.6653932865284646e-07,
    4.1471802207647471e-07,
    4.6922942339406725e-07,
    5.3090591693198118e-07,
    6.0068929734757113e-07,
    6.7964515075115804e-07,
    7.6897912611266348e-07,
    8.7005534541583277e-07,
    9.8441723368152588e-07,
    1.1138110869326676e-06,
    1.2602127379816654e-06,
    1.4258577272244814e-06,
    1.6132754391468040e-06,
    1.8253277258037093e-06,
    2.0652526070500441e-06,
    2.3367137148201549e-06,
    2.6438562364668111e-06,
    2.9913702114092511e-06,
    3.3845621476245273e-06,
    3.8294360515598428e-06,
    4.3327851087853783e-06,
    4.9022954153433495e-06,
    5.5466633437616938e-06,
    6.2757283359013852e-06,
    7.1006231503722614e-06,
    8.0339438587825533e-06,
    9.0899421866496263e-06,
    1.0284743136001185e-05,
    1.1636591212744581e-05,
    1.3166129018675088e-05,
    1.4896712462198172e-05,
    1.6854767401006508e-05,
    1.9070193162613521e-05,
    2.1576819104467435e-05,
    2.4412921185277932e-05,
    2.7621806435555346e-05,
    3.1252474252175486e-05,
    3.5360364614880511e-05,
    4.0008204649916913e-05,
    4.5266966467761903e-05,
    5.1216950901038227e-05,
    5.7949013691198461e-05,
    6.5565952847744405e-05,
    7.4184078364832791e-05,
    8.3934988264705517e-05,
    9.4967578087968609e-05,
    1.0745031451546452e-04,
    1.2157380783974028e-04,
    1.3755372256753069e-04,
    1.5563407059789586e-04,
    1.7609093726256181e-04,
    1.9923669712476597e-04,
    2.2542478391149605e-04,
    2.5505508741555978e-04,
    2.8858005977771921e-04,
    3.2651162439127017e-04,
    3.6942899292744950e-04,
    4.1798750984694958e-04,
    4.7292865945246747e-04,
    5.3509138828909957e-04,
    6.0542491578464660e-04,
    6.8500322874718428e-04,
    7.7504148104960886e-04,
    8.7691454892290584e-04,
    9.9217802519584457e-04,
    1.1225919730613013e-03,
    1.2701478020871444e-03,
    1.4370986768660126e-03,
    1.6259939226414136e-03,
    1.8397179532810260e-03,
    2.0815343160239703e-03,
    2.3551355255614684e-03,
    2.6646994484129465e-03,
    3.0149530985820704e-03,
    3.4112448186467913e-03,
    3.8596259484823358e-03,
    4.3669432286913834e-03,
    4.9409433497337261e-03,
    5.5903912432114702e-03,
    6.3252039216073431e-03,
    7.1566019101972009e-03,
    8.0972805834887800e-03,
    9.1616040224791161e-03,
    1.0365824352913980e-02,
    1.1728329913825357e-02,
    1.3269926045858802e-02,
    1.5014152787003869e-02,
    1.6987644326913583e-02,
    1.9220535708648952e-02,
    2.1746922988148634e-02,
    2.4605383878018375e-02,
    2.7839566825825592e-02,
    3.1498857522072835e-02,
    3.5639133015368717e-02,
    4.0323614950068816e-02,
    4.5623834955250860e-02,
    5.1620726926428946e-02,
    5.8405862879053430e-02,
    6.6082851245170882e-02,
    7.4768918965111897e-02,
    8.4596701532608246e-02,
    9.5716268327171616e-02,
    1.0829741416038222e-01,
    1.2253225202779826e-01,
    1.3863814665758106e-01,
    1.5686103365086684e-01,
    1.7747917489686738e-01,
    2.0080740760756019e-01,
    2.2720195185435565e-01,
    2.5706585001740484e-01,
    2.9085512120746343e-01,
    3.2908572463779440e-01,
    3.7234143827618899e-01,
    4.2128277308343309e-01,
    4.7665705895785876e-01,
    5.3930985639747342e-01,
    6.1019786813474608e-01,
    6.9040354790358571e-01,
    7.8115162941310323e-01,
    8.8382782792413994e-01,
    1.0000000000000000e+00,
};
#endif

static inline void activate_array(float *x, const int n, const INPUT_TYPE *input)
{

    for (int i = 0; i < n; ++i)
#ifdef DEBUG_FLOAT
        x[i] = 1.0f / (1.0f + expf(-input[i]));
#else
        x[i] = activate_array_acc[input[i]];
#endif
}

static inline int entry_index(int location, int entry)
{
    // LOG("_start %s [region_layer] start run\n", __func__);

    int n = location / (region_layer_l_w * region_layer_l_h);
    int loc = location % (region_layer_l_w * region_layer_l_h);

    return n * region_layer_l_w * region_layer_l_h *
               (region_layer_l_coords + region_layer_l_classes + 1) +
           entry * region_layer_l_w * region_layer_l_h + loc;
}

static inline void softmax(const INPUT_TYPE *u8in, int n, int stride, float *output)
{
    // LOG("_start %s [region_layer] start run\n", __func__);

    int i;
    float e;
    float sum = 0;
    INPUT_TYPE largest_i = u8in[0];
#ifdef DEBUG_FLOAT
    float diff;
#else
    int diff;
#endif

    for (i = 0; i < n; ++i)
    {
        if (u8in[i * stride] > largest_i)
            largest_i = u8in[i * stride];
    }

    for (i = 0; i < n; ++i)
    {
        diff = u8in[i * stride] - largest_i;
#ifdef DEBUG_FLOAT
        e = expf(diff);
#else
        e = softmax_acc[diff + 255];
#endif
        sum += e;
        output[i * stride] = e;
    }
    for (i = 0; i < n; ++i)
        output[i * stride] /= sum;
    // LOG("_end %s [region_layer] end run\n", __func__);
}

static inline void softmax_cpu(const INPUT_TYPE *input, int n, int batch, int batch_offset, int groups, int stride, float *output)
{
    // LOG("_start %s [region_layer] start run\n", __func__);

    int g, b;

    for (b = 0; b < batch; ++b)
    {
        for (g = 0; g < groups; ++g)
            softmax(input + b * batch_offset + g, n, stride, output + b * batch_offset + g);
    }
    // LOG("_end %s [region_layer] end run\n", __func__);
}
static inline void forward_region_layer(const INPUT_TYPE *u8in, float *output)
{
    LOG("_start %s [region_layer] start run\n", __func__);

    volatile int n, index;

    for (index = 0; index < 8750; index++)
        output[index] = u8in[index] * scale + bais;

    for (n = 0; n < region_layer_l_n; ++n)
    {
        index = entry_index(n * region_layer_l_w * region_layer_l_h, 0);
        activate_array(output + index, 2 * region_layer_l_w * region_layer_l_h, u8in + index);
        index = entry_index(n * region_layer_l_w * region_layer_l_h, 4);
        activate_array(output + index, region_layer_l_w * region_layer_l_h, u8in + index);
    }

    index = entry_index(0, 5);
    softmax_cpu(u8in + index, region_layer_l_classes, region_layer_l_n,
                region_layer_outputs / region_layer_l_n,
                region_layer_l_w * region_layer_l_h,
                region_layer_l_w * region_layer_l_h, output + index);
    LOG("_end %s [region_layer] end run\n", __func__);
}

static inline void correct_region_boxes(box *boxes)
{
    // LOG("_start %s [region_layer] start run\n", __func__);

    int new_w = 0;
    int new_h = 0;

    if (((float)region_layer_net_w / region_layer_img_w) <
        ((float)region_layer_net_h / region_layer_img_h))
    {
        new_w = region_layer_net_w;
        new_h = (region_layer_img_h * region_layer_net_w) / region_layer_img_w;
    }
    else
    {
        new_h = region_layer_net_h;
        new_w = (region_layer_img_w * region_layer_net_h) / region_layer_img_h;
    }
    for (int i = 0; i < region_layer_boxes; ++i)
    {
        volatile box b = boxes[i];

        b.x = (b.x - (region_layer_net_w - new_w) / 2. / region_layer_net_w) /
              ((float)new_w / region_layer_net_w);
        b.y = (b.y - (region_layer_net_h - new_h) / 2. / region_layer_net_h) /
              ((float)new_h / region_layer_net_h);
        b.w *= (float)region_layer_net_w / new_w;
        b.h *= (float)region_layer_net_h / new_h;
        boxes[i] = b;
    }
}

static inline box get_region_box(float *x, const float *biases, int n, int index, int i, int j, int w, int h, int stride)
{
    // LOG("_start %s [region_layer] start run\n", __func__);

    volatile box b;

    b.x = (i + x[index + 0 * stride]) / w;
    b.y = (j + x[index + 1 * stride]) / h;
    b.w = expf(x[index + 2 * stride]) * biases[2 * n] / w;
    b.h = expf(x[index + 3 * stride]) * biases[2 * n + 1] / h;
    return b;
}

static inline void get_region_boxes(float *predictions, float **probs, box *boxes)
{
    // LOG("_start %s [region_layer] start run\n", __func__);

    for (int i = 0; i < region_layer_l_w * region_layer_l_h; ++i)
    {
        volatile int row = i / region_layer_l_w;
        volatile int col = i % region_layer_l_w;

        for (int n = 0; n < region_layer_l_n; ++n)
        {
            int index = n * region_layer_l_w * region_layer_l_h + i;

            for (int j = 0; j < region_layer_l_classes; ++j)
                probs[index][j] = 0;
            int obj_index = entry_index(n * region_layer_l_w * region_layer_l_h + i, 4);
            int box_index = entry_index(n * region_layer_l_w * region_layer_l_h + i, 0);
            float scale = predictions[obj_index];

            boxes[index] = get_region_box(
                predictions, l_biases, n, box_index, col, row,
                region_layer_l_w, region_layer_l_h,
                region_layer_l_w * region_layer_l_h);

            float max = 0;

            for (int j = 0; j < region_layer_l_classes; ++j)
            {
                int class_index = entry_index(n * region_layer_l_w * region_layer_l_h + i, 5 + j);
                float prob = scale * predictions[class_index];
                // cprintf("prob = scale * predictions[class_index] :%d \n", prob);
                probs[index][j] = (prob > region_layer_thresh) ? prob : 0;
                if (prob > max)
                    max = prob;
            }
            probs[index][region_layer_l_classes] = max;
        }
    }
    correct_region_boxes(boxes);
}

static inline int nms_comparator(const void *pa, const void *pb)
{

    // LOG("_start %s [region_layer] start run\n", __func__);

    volatile sortable_bbox a = *(sortable_bbox *)pa;
    volatile sortable_bbox b = *(sortable_bbox *)pb;
    volatile float diff = a.probs[a.index][b.classes] - b.probs[b.index][b.classes];

    if (diff < 0)
        return 1;
    else if (diff > 0)
        return -1;
    return 0;
}

static inline float overlap(float x1, float w1, float x2, float w2)
{
    // LOG("_start %s [region_layer] start run\n", __func__);

    float l1 = x1 - w1 / 2;
    float l2 = x2 - w2 / 2;
    float left = l1 > l2 ? l1 : l2;
    float r1 = x1 + w1 / 2;
    float r2 = x2 + w2 / 2;
    float right = r1 < r2 ? r1 : r2;

    return right - left;
}

static inline float box_intersection(box a, box b)
{
    // LOG("_start %s [region_layer] start run\n", __func__);

    float w = overlap(a.x, a.w, b.x, b.w);
    float h = overlap(a.y, a.h, b.y, b.h);

    if (w < 0 || h < 0)
        return 0;
    return w * h;
}

static inline float box_union(box a, box b)
{
    // LOG("_start %s [region_layer] start run\n", __func__);

    float i = box_intersection(a, b);
    float u = a.w * a.h + b.w * b.h - i;

    return u;
}

static inline float box_iou(box a, box b)
{
    // LOG("_start %s [region_layer] start run\n", __func__);

    return box_intersection(a, b) / box_union(a, b);
}

static inline void do_nms_sort(box *boxes, float **probs)
{
    // LOG("_start %s [region_layer] start run\n", __func__);

    int i, j, k;
    sortable_bbox s[region_layer_boxes];

    for (i = 0; i < region_layer_boxes; ++i)
    {
        s[i].index = i;
        s[i].classes = 0;
        s[i].probs = probs;
    }

    for (k = 0; k < region_layer_l_classes; ++k)
    {
        for (i = 0; i < region_layer_boxes; ++i)
            s[i].classes = k;
        qsort(s, region_layer_boxes, sizeof(sortable_bbox), nms_comparator);
        for (i = 0; i < region_layer_boxes; ++i)
        {
            if (probs[s[i].index][k] == 0)
                continue;
            volatile box a = boxes[s[i].index];

            for (j = i + 1; j < region_layer_boxes; ++j)
            {
                volatile box b = boxes[s[j].index];

                if (box_iou(a, b) > region_layer_nms)
                    probs[s[j].index][k] = 0;
            }
        }
    }
}

static inline int max_index(float *a, int n)
{
    // LOG("_start %s [region_layer] start run\n", __func__);

    int i, max_i = 0;
    float max = a[0];

    for (i = 1; i < n; ++i)
    {
        if (a[i] > max)
        {
            max = a[i];
            max_i = i;
        }
    }
    return max_i;
}

static float output[region_layer_outputs];
static box boxes[region_layer_boxes];
static float probs_buf[region_layer_boxes * (region_layer_l_classes + 1)];
static float *probs[region_layer_boxes];

void region_layer_cal(INPUT_TYPE *u8in)
{
    LOG("_start %s [region_layer] start run\n", __func__);

    forward_region_layer(u8in, output);

    for (int i = 0; i < region_layer_boxes; i++)
        probs[i] = &(probs_buf[i * (region_layer_l_classes + 1)]);

    get_region_boxes(output, probs, boxes);
    do_nms_sort(boxes, probs);
}

void region_layer_draw_boxes(callback_draw_box callback)
{
    LOG("_start %s [region_layer] start run\n", __func__);

    for (int i = 0; i < region_layer_boxes; ++i)
    {
        volatile int classe = max_index(probs[i], region_layer_l_classes);
        volatile float prob = probs[i][classe];
        // cprintf("PROB: %d\n", (int)prob);
        if (prob > region_layer_thresh)
        {
            volatile box *b = boxes + i;
            uint32_t x1 = b->x * region_layer_img_w -
                          (b->w * region_layer_img_w / 2);
            uint32_t y1 = b->y * region_layer_img_h -
                          (b->h * region_layer_img_h / 2);
            uint32_t x2 = b->x * region_layer_img_w +
                          (b->w * region_layer_img_w / 2);
            uint32_t y2 = b->y * region_layer_img_h +
                          (b->h * region_layer_img_h / 2);

            callback(x1, y1, x2, y2, classe, prob);
        }
    }
    LOG("_end %s [region_layer] end run\n", __func__);
}
