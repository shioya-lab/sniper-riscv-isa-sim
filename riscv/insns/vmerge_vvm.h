// vmerge.vvm vd, vs2, vs1
VI_VV_MERGE_LOOP
({
  vd = use_first ? vs1 : vs2;
})
P.get_state()->mhpmcounter[10]->bump(1);
