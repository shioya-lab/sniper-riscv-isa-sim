// vsse64.v and vssseg[2-8]e64.v
VI_ST(i * RS2, fn, uint64, false);
P.get_state()->mhpmcounter[10]->bump(1);
