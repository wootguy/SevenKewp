// header for calling rehlds functions without including rehlds headers, which may cause conflicts

struct PerfMetrics;
typedef struct model_s model_t;

void rehlds_set_perf_timings(PerfMetrics* metrics);

void rehlds_init();

model_t* rehlds_get_model(int modelidx);