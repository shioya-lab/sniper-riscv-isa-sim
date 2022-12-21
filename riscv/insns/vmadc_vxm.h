// vadc.vx vd, vs2, rs1, v0
VI_XI_LOOP_CARRY
({
  res = (((op_mask & rs1) + (op_mask & vs2) + carry) >> sew) & 0x1u;
})
P.get_state()->mhpmcounter[10]->bump(1);
