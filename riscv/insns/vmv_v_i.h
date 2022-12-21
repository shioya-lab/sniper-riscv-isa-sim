// vmv.v.i vd, simm5
VI_VI_MERGE_LOOP
({
  vd = simm5;
})
P.get_state()->mhpmcounter[10]->bump(1);
