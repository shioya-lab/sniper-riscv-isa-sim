// vfsub.vf vd, vs2, rs1
VI_VFP_VF_LOOP
({
  vd = f16_sub(rs1, vs2);
},
{
  vd = f32_sub(rs1, vs2);
},
{
  vd = f64_sub(rs1, vs2);
})
P.get_state()->mhpmcounter[10]->bump(1);
