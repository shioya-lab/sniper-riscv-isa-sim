// vfmv_vf vd, vs1
VI_VF_MERGE_LOOP({
  vd = rs1;
})
P.get_state()->mhpmcounter[10]->bump(1);
