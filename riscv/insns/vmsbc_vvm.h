// vmsbc.vvm vd, vs2, rs1, v0
VI_VV_LOOP_CARRY
({
  res = (((op_mask & vs2) - (op_mask & vs1) - carry) >> sew) & 0x1u;
})
P.get_state()->mhpmcounter[10]->bump(1);
