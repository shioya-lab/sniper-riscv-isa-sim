//vamominue.v vd, (rs1), vs2, vd
VI_AMO({ return lhs < vs3 ? lhs : vs3;; }, uint, e64);
P.get_state()->mhpmcounter[10]->bump(1);
