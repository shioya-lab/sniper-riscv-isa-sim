// vseq.vx vd, vs2, rs1
VI_VX_LOOP_CMP
({
  res = rs1 == vs2;
})
P.get_state()->mhpmcounter[10]->bump(1);
