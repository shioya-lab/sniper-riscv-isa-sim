// vlse32.v and vlsseg[2-8]e32.v
VI_LD(i * RS2, fn, int32, false);
P.get_state()->mhpmcounter[10]->bump(1);
