#pragma once

void skartveit(double *indices_glo, double index_beam, int sph, double previous_ligoh, double *indices_glo_st, double *actual_ligoh);
void estimate_sigmas(double *indices_glo, double index_beam, int sph, double *sigma_glo, double *sigma_beam);
void estimate_indices_glo_st(double index_glo, double index_beam, int sph, double sigma_glo, double previous_ligoh, \
	int *glo_ranking, double *indices_glo_st, double *actual_ligoh);

