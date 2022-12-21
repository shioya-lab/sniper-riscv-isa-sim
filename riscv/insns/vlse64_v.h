// vlse64.v and vlsseg[2-8]e64.v
VI_LD(i * RS2, fn, int64, false);
P.get_state()->mhpmcounter[10]->bump(1);
