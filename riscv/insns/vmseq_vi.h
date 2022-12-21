// vseq.vi vd, vs2, simm5
VI_VI_LOOP_CMP
({
  res = simm5 == vs2;
})
P.get_state()->mhpmcounter[10]->bump(1);
