// �R�}���h��`
typedef struct command_t {
    int (*func)(int, char**);
    char *name; // ���O
    char *desc; // ����
} command;

#define EXEC_COMMAND_NOT_FOUND -1

#define SERVO_CH 2
#define SERVO_TAIL_VERTICAL 0
#define SERVO_TAIL_HORIZONTAL 1

#define SERVO_DEG(x) (int)((x) * 100)

#define CONFIG_SIGNATURE "CFG0"
#define CONFIG_SIGNATURE_LENGTH 4

typedef struct servo_t {
    int offset; // �T�[�{�̒����ʒu�D����͊p�x�ł͂Ȃ��w�ߒl
    int rot_count; // 1��]�ɂ�����p���X��
    int invert; // 1 (invert) or 0 (not invert)
    int trim; // 0.01�x�P�� -180.00�`+180.00
    int range; // 0.01�x�P�� 0.00�`360.00
    int limit_upper; // 0.01�x�P��, -180.00�`+180.00
    int limit_lower; // 0.01�x�P��, -180.00�`+180.00
} servo_cfg;

typedef struct config_pack_t {
    // header
    char signature[CONFIG_SIGNATURE_LENGTH]; // "CFG0"
    int size; // sizeof(config)
    // data
    servo_cfg servo[SERVO_CH];
    // footer
    unsigned char checksum; // checksum without footer
} config_pack;

// prototype
int exec(char *cmdline);
void hash_bar(int val, int max);
// command
int help(int argc, char *argv[]);
int clear(int argc, char *argv[]);
int reset(int argc, char *argv[]);
int resf(int argc, char *argv[]);
int load(int argc, char *argv[]);
int save(int argc, char *argv[]);
int default_config(int argc, char *argv[]);
int led(int argc, char *argv[]);
int ad(int argc, char *argv[]);
int control(int argc, char *argv[]);
int servo_pos(int argc, char *argv[]);
int servo_set(int argc, char *argv[]);
int servo_writeid(int argc, char *argv[]);
int show(int argc, char *argv[]);

