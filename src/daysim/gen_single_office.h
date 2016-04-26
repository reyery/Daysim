char  *progname;
int close_file(FILE *f);
FILE *open_output(char *filename);
FILE *open_input(char *filename);
double transmissivity(double visual_transmittance);
void writeErrorMessage();
void checkPassedArguements(int argc, char *argv[]);
void writeVFFile();
void writeRadianceHeaderFile(int argc, char  *argv[]);
void writeCopyrightHeader();
void writeGeneralOfficeParameters();
void writeMaterialDescription();
void writeSkyDescription();
void writeOfficeGeometry();
void writeFloorGeometry();
void writeCeilingGeometry();
void writeOfficeFacadeGeometry(int i);
void writeOfficeWallGeometry();
void setOfficeOrientation();
void writeBuildingGeometry(int i);
void writeBuildingFacadeGeometry(int i);
void writeSurroundings();
void writePavementGeometry();
void writeContinuousObstBuildings();
void writeDiscontinuousObstBuildings();
void writeWhiteGround();

double orientation = 0.0;
FILE* RAD_FILE;
char rad_file[200]="";
char pts_file[200]="";
char bin_dir[200]="";
FILE* VF_FILE;
FILE* VF_FILE1;
FILE* VF_FILE2;
FILE* VF_FILE3;
char vf_file[200]="";
char vf_file1[200]="";
char vf_file2[200]="";
char vf_file3[200]="";
FILE* LAMP_MAT;
char lamp_mat[200]="";
char befehl[1024]="";
FILE *ORIENT;
float VDT_distance_to_facade=0;
float VDT_distance_to_wall=0;
float x,y,z,dir_x,dir_y,dir_z;
float x_tmp,y_tmp,angle;
int number_of_neighbors=1;
int add_sky=0;

double points_height = 0.85;

int  first_line=0;
int make_vf=0,make_points=0;
int add_lamp=0;
int facade_type=1;
int seating_position=0;
int addWiteGroundPlate=0;
int ballustrade=1;
double window_width = 1;

double darkerwall = 1.0;
double ceiling_height = 2.74;/* 9ft */
double office_width = 3.048;/* 10ft */
double office_width_ft = 10;
double office_depth = 4.572;/* 15ft */
double office_depth_ft = 15;/* 15 ft */

double floor_height = 3.048;/*10ft default*/
double building_depth = 45.72;/*150ft*/
double building_depth_ft = 150;
double building_width = 45.72;/*150ft*/
double building_width_ft = 150;
int num_building_floors = 10;
double building_height = 30.48;/*100 ft*/
double building_height_ft = 100;
int office_level = 1;

double street_width = 18.288;
double street_width_ft = 60;


double pavement_width = 298.704;
double pavement_width_ft = 980;
double pavement_depth = 298.704;
double pavement_depth_ft = 980;

double frame_width = 0.1016; /* 4" */
double sill = 0.0254; /* 1" */


int month=6;
int day=21;
float hour=12.0;
double ceiling_refl = 0.8;
double wall_refl = 0.5;
double floor_refl = 0.2;
double visual_transmittance = 0.8;
double glazed_partition = 0;
double glazed_partition_trans = 0;
double obstruction_angle = 0;
int levels_above_office=0;
double street_refl = .3;
double street_specul = 0.00;
double street_roughness = 0.00;
double facade_refl = .3;

int obstruction=0;
double building_refl = .3;
int obstruction_type=0;
double obstruction_building_depth = 45.72;/*300ft*/
double obstruction_building_depth_ft = 150;
double obstruction_building_width = 45.72;/*150ft*/
double obstruction_building_width_ft = 150;

double obstruction_building_height = 30.48;/*100 ft*/
double obstruction_building_height_ft = 100;/*100ft*/

double spacing_btw_discon_buildings_ft = 30;
double spacing_btw_discon_buildings = 9.144;
double discont_building_width_ft = 30;
double discont_building_width = 9.144;
double discont_building_depth_ft = 30;
double discont_building_depth = 9.144;
