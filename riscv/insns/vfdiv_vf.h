// vfdiv.vf vd, vs2, rs1
VI_VFP_VF_LOOP
({
  vd = f16_div(vs2, rs1);
},
{
  vd = f32_div(vs2, rs1);
},
{
  vd = f64_div(vs2, rs1);
})
P.get_state()->mhpmcounter[10]->bump(1);
