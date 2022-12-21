// vfwsub.vf vd, vs2, rs1
VI_VFP_VF_LOOP_WIDE
({
  vd = f32_sub(vs2, rs1);
},
{
  vd = f64_sub(vs2, rs1);
})
P.get_state()->mhpmcounter[10]->bump(1);
