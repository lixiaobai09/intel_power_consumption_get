#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h>
#define N 1024
typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;

static char buf[N];
typedef struct rapl_power_unit{
    double PU;       //power units
    double ESU;      //energy status units
    double TU;       //time units
} rapl_power_unit;
//this struct is not usefull now
//typedef struct pkg_power_info{
//    double TSP;      //thermal specification power
//    double MinP;     //minimum power
//    double MaxP;     //maximum power
//    double MaxTW;    //maximum time window
//} pkg_power_info;

uint64_t rdmsr(int cpu, uint32_t reg) {
    sprintf(buf, "/dev/cpu/%d/msr", cpu);
    int msr_file = open(buf, O_RDONLY);
    if (msr_file < 0) {
        perror("rdmsr: open");
        return msr_file;
    }
    uint64_t data;
    if ( pread(msr_file, &data, sizeof(data), reg) != sizeof(data)) {
        fprintf(stderr, "read msr register 0x%x error.\n", reg);
        perror("rdmsr: read msr");
        return -1;
    }
    close(msr_file);
    return data;
}

rapl_power_unit get_rapl_power_unit() {
    rapl_power_unit ret;
    uint64_t data = rdmsr(0, 0x606);
    double t = (1 << (data & 0xf));
    t = 1.0 / t;
    ret.PU = t;
    t = (1 << ((data>>8) & 0xf));
    ret.ESU = 1.0 / t;
    t = (1 << ((data>>16) & 0xf));
    ret.TU = 1.0 / t;
    return ret;
}

double get_cpu_power(int n, int* cpus, double energy_units, int cycle) {   // cycle with unit ms
    int cpu, i;
    uint64_t data;
    double *st, *en, *count;
    st = malloc(n*sizeof(double));
    en = malloc(n*sizeof(double));
    count = malloc(n*sizeof(double));
    for (i=0; i<n; ++i) {
        cpu = cpus[i];
        data = rdmsr(cpu, 0x611);
        st[i] = (data & 0xffffffff) * energy_units;
    }
    usleep(cycle*1000);
    for (i=0; i<n; ++i) {
        cpu = cpus[i];
        data = rdmsr(cpu, 0x611);
        en[i] = (data & 0xffffffff) * energy_units;
        count[i] = 0;
        if (en[i] < st[i]) {
            count[i] = (double)(1ll << 32) + en[i] - st[i];
        }
        else {
            count[i] = en[i] - st[i];
        }
        count[i] = count[i] / ((double)cycle) * 1000.0;
    }
    double ret = 0.0;
    for (i=0; i<n; ++i) {
        cpu = cpus[i];
        printf("get cpu %d power comsumption is: %f\n", cpu, count[i]);
        ret += count[i];
    }
    free(st);
    free(en);
    free(count);
    return ret;
}

struct option longOpt[] = {
    {"time",  required_argument, NULL, 't'},
    {"cpu",   required_argument, NULL, 'c'},
    {"help",        no_argument, NULL, 'h'},
    {NULL,          no_argument, NULL, 0}
};
static const char* optString = "t:c:h";
void usage(const char* program_name) {
    printf("usage: %s option argument\n", program_name);
    printf("       -t --time [time] set the cycle of get power, unit is ms.\n");
    printf("       -c --cpu  [cpu id] add the cpu to the cpu list.\n");
    printf("       -h --help print this help infomation.\n");
    printf("       This program is used to calculate the total power comsumption in cpu list. This program needs root privillege.\n");
    exit(0);
}

static int cpus[64];
int main(int argc, char** argv) {
    int opt, longIndex;
    int cpu, cycle;
    int n = 0;
    opt = getopt_long(argc, argv, optString, longOpt, &longIndex);
    if (opt == -1) {
        usage(argv[0]);
    }
    while (opt != -1) {
        switch(opt) {
            case 't':
                cycle = atoi(optarg);
                break;
            case 'c':
                cpu = atoi(optarg);
                cpus[n++] = cpu;
                break;
            case 'h':
            case 0:
            default:
                usage(argv[0]);
        }
        opt = getopt_long(argc, argv, optString, longOpt, &longIndex);
    }
    rapl_power_unit power_units = get_rapl_power_unit();
    while (1) {
        printf("total power consumption of cpu list is %f\n", get_cpu_power(n, cpus, power_units.ESU, cycle));
    }
    return 0;
}
