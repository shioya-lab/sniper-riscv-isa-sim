// vsse32.v and vssseg[2-8]e32.v
VI_ST(i * RS2, fn, uint32, false);
P.get_state()->mhpmcounter[10]->bump(1);
