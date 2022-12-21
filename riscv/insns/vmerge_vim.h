// vmerge.vim vd, vs2, simm5
VI_VI_MERGE_LOOP
({
  vd = use_first ? simm5 : vs2;
})
P.get_state()->mhpmcounter[10]->bump(1);
