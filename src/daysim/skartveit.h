#pragma once

void skartveit(double *indices_glo, double index_beam, int sph, double previous_ligoh, double *indices_glo_st, double *actual_ligoh);
void estimate_sigmas(float *indices_glo, float index_beam, int sph, float *sigma_glo, float *sigma_beam);
void estimate_indices_glo_st ( float index_glo, float index_beam, int sph, float sigma_glo, float previous_ligoh, int *glo_ranking,\
                               float *indices_glo_st, float *actual_ligoh );
