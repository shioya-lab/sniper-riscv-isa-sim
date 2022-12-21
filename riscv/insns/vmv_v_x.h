// vmv.v.x vd, rs1
VI_VX_MERGE_LOOP
({
  vd = rs1;
})
P.get_state()->mhpmcounter[10]->bump(1);
