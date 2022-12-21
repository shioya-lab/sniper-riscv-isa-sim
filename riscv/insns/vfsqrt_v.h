// vsqrt.v vd, vd2, vm
VI_VFP_V_LOOP
({
  vd = f16_sqrt(vs2);
},
{
  vd = f32_sqrt(vs2);
},
{
  vd = f64_sqrt(vs2);
})
P.get_state()->mhpmcounter[10]->bump(1);
