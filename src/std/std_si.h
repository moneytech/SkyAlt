/*
 * Copyright (c) 2018 Milan Suk
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2024-11-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

double SI_mass_lb_oz(double lb)
{
	return lb * 16;
}

double SI_mass_ton_oz(double ton)
{
	return ton * (16 * 2000);
}

double SI_mass_kg_g(double kg)
{
	return kg * 1000;
}

double SI_mass_t_g(double t)
{
	return t * (1000 * 1000);
}

double SI_mass_cross_g_oz(double g)
{
	return g / 28.349523125;
}

double SI_mass_cross_oz_g(double oz)
{
	return oz * 28.349523125;
}

UNI* SI_mass_oz(double oz)
{
	UNI* str;
	if(oz >= 16 * 2000)
		str = Std_addAfterUNI_char(Std_newNumber(oz / (16 * 2000)), " ton"); //ton
	else
		if(oz >= 16)
		str = Std_addAfterUNI_char(Std_newNumber(oz / 16), " lb"); //pound
	else
		str = Std_addAfterUNI_char(Std_newNumber(oz), " oz"); //ounce 

	return str;
}

UNI* SI_mass_g(double g)
{
	UNI* str;
	if(g >= 1000 * 1000)
		str = Std_addAfterUNI_char(Std_newNumber(g / (1000 * 1000)), " t"); //tonne
	else
		if(g >= 1000)
		str = Std_addAfterUNI_char(Std_newNumber(g / 1000), " kg"); //kilogram
	else
		str = Std_addAfterUNI_char(Std_newNumber(g), " g"); //gram

	return str;
}

/*LENGTH*/
double SI_length_in_p(double in)
{
	return in * 6;
}

double SI_length_ft_p(double ft)
{
	return ft * (12 * 6);
}

double SI_length_yd_p(double yd)
{
	return yd * (3 * 12 * 6);
}

double SI_length_mi_p(double mi)
{
	return mi * (1760 * 3 * 12 * 6);
}

double SI_length_cm_mm(double cm)
{
	return cm * 10;
}

double SI_length_m_mm(double m)
{
	return m * (100 * 10);
}

double SI_length_km_mm(double km)
{
	return km * (1000 * 100 * 10);
}

double SI_length__cross_mm_p(double mm)
{
	return mm / 4.233;
}

double SI_length__cross_p_mm(double p)
{
	return p * 4.233;
}

UNI* SI_length_p(double p)
{
	UNI* str;
	if(p >= 1760 * 3 * 12 * 6)
		str = Std_addAfterUNI_char(Std_newNumber(p / (1760 * 3 * 12 * 6)), " mi");
	else
		if(p >= 3 * 12 * 6)
		str = Std_addAfterUNI_char(Std_newNumber(p / (3 * 12 * 6)), " yd");
	else
		if(p >= 12 * 6)
		str = Std_addAfterUNI_char(Std_newNumber(p / (12 * 6)), " ft");
	else
		if(p >= 6)
		str = Std_addAfterUNI_char(Std_newNumber(p / 6), " in");
	else
		str = Std_addAfterUNI_char(Std_newNumber(p), " P");

	return str;
}

UNI* SI_length_mm(double mm)
{
	UNI* str;
	if(mm >= 1000 * 100 * 10)
		str = Std_addAfterUNI_char(Std_newNumber(mm / (3 * 12 * 6)), " km");
	else
		if(mm >= 100 * 10)
		str = Std_addAfterUNI_char(Std_newNumber(mm / (12 * 6)), " m");
	else
		if(mm >= 10)
		str = Std_addAfterUNI_char(Std_newNumber(mm / 6), " cm");
	else
		str = Std_addAfterUNI_char(Std_newNumber(mm), " mm");

	return str;
}

/*POWER*/
double SI_power_kw_hp_us(double kw)
{
	return kw * 1.3404825737;
}

double SI_power_kw_hp_eu(double kw)
{
	return kw * 1.3605442177;
}

double SI_power_hp_kw_us(double hp)
{
	return hp / 1.3404825737;
}

double SI_power_hp_kw_eu(double hp)
{
	return hp / 1.3605442177;
}

