char  *progname;
int close_file(FILE *f);
FILE *open_output(char *filename);
FILE *open_input(char *filename);
float transmissivity(float visual_transmittance);
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

float orientation=0.0;
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

float points_height=0.85;

int  first_line=0;
int make_vf=0,make_points=0;
int add_lamp=0;
int facade_type=1;
int seating_position=0;
int addWiteGroundPlate=0;
int ballustrade=1;
float window_width=1;

float darkerwall=1.0;
float ceiling_height=2.74;/* 9ft */
float office_width=3.048;/* 10ft */
float office_width_ft=10;
float office_depth=4.572;/* 15ft */
float office_depth_ft=15;/* 15 ft */

float floor_height = 3.048;/*10ft default*/
float building_depth = 45.72;/*150ft*/
float building_depth_ft = 150;
float building_width = 45.72;/*150ft*/
float building_width_ft = 150;
int num_building_floors = 10;
float building_height = 30.48;/*100 ft*/
float building_height_ft = 100;
int office_level = 1;

float street_width=18.288;
float street_width_ft=60;


float pavement_width =  298.704 ;
float pavement_width_ft = 980;
float pavement_depth =  298.704 ;
float pavement_depth_ft = 980;

float frame_width=0.1016; /* 4" */
float sill=0.0254; /* 1" */


int month=6;
int day=21;
float hour=12.0;
float ceiling_refl=0.8;
float wall_refl=0.5;
float floor_refl=0.2;
float visual_transmittance=0.8;
float glazed_partition=0;
float glazed_partition_trans=0;
float obstruction_angle=0;
int levels_above_office=0;
float street_refl=.3;
float street_specul = 0.00;
float street_roughness = 0.00;
float facade_refl=.3;

int obstruction=0;
float building_refl=.3;
int obstruction_type=0;
float obstruction_building_depth = 45.72;/*300ft*/
float obstruction_building_depth_ft = 150;
float obstruction_building_width = 45.72;/*150ft*/
float obstruction_building_width_ft = 150;

float obstruction_building_height = 30.48;/*100 ft*/
float obstruction_building_height_ft = 100;/*100ft*/

float spacing_btw_discon_buildings_ft = 30;
float spacing_btw_discon_buildings = 9.144;
float discont_building_width_ft= 30;
float discont_building_width= 9.144;
float discont_building_depth_ft= 30;
float discont_building_depth= 9.144;
