/**
  ******************************************************************************
  * @file   
  * @brief   
  * @date    2022.09.13
  ******************************************************************************
  * @attention  
  1.����onchip-flash ����SPI-FLSAH  
  2.����SFUD ����FAL���
  3.���� <fal_cfg.h> �����
  4.��������ļ�ϵͳDFS�������MFD
  5.���elm fatfs�ļ�ϵͳ��
  6.romfs.c��toolĿ¼���½�romfs�ļ��в��Զ�������һ��Ŀ¼�����ʹ��mkromfs.py����
  python mkromfs.py romfs romfs.c

  7.����LOGSĿ¼
  
  ����ULOG��־����λ���У�
      1.cd��ulog�ļ�����Ŀ¼ cd/flash/logs
      2.sy ulog.log
      ��ʾ��No permission to write on the specified folder.��Ϊ��"/"�����޷�����
  ����λ��������������mcu��
      1.xshell����ry�ȿɡ�Ĭ�Ͻ�����/flashĿ¼��
      
  Ƭ��onchipʹ��littlefsϵͳ����������Ŀ¼�󼰱���ռ䲻�㡣
  ���ļ�ϵͳ�ṹ�������������㡣��Ȼ�󣬵�����������
  littlefsд���ٶ������fatfs���������������������fatfs.fatfsû�е��籣�����дƽ�⡣
  �������BAD file sys �ļ����Ļ�littlefs
  
  fatfs�ļ�ϵͳ��ע�⣬��������������ļ��𻵣���Ҫ���¸�ʽ�� ���� mkfs -t elm filesystem
  
  ����CMB����־�ļ����档��һ��ֻ�ܶԷ������в�����û�а취���浽����Ŀ¼�¡�
  �ڶ���ʹ��rom������Ŀ¼�󣬴���һ�������޷�ʹ��littlefs�ļ�ϵͳ��ʽ����ֻ����romϵͳ��ֻ�����ԣ�����д�롣
  * @author 
  ******************************************************************************
  */
/*
msh /flash/logs>fal bench 4096
DANGER: It will erase full chip or partition! Please run 'fal bench 4096 yes'.
msh /flash/logs>fal bench 4096 yes
Erasing 1048576 bytes data, waiting...
Erase benchmark success, total time: 3.548S.
Writing 1048576 bytes data, waiting...
Write benchmark success, total time: 4.097S.
Reading 1048576 bytes data, waiting...
Read benchmark success, total time: 1.546S.

Erase benchmark success, total time: 33.946S.
Writing the sf_cmd 16777216 bytes data, waiting...
Write benchmark success, total time: 65.537S.
Reading the sf_cmd 16777216 bytes data, waiting...
Read benchmark success, total time: 26.302S.

��������
ls	��ʾ�ļ���Ŀ¼����Ϣ
cd	����ָ��Ŀ¼
cp	�����ļ�
rm	ɾ���ļ���Ŀ¼ //�ļ�����������ɾ����������ʹ�� rm-rf
mv	���ļ��ƶ�λ�û����
echo	��ָ������д��ָ���ļ������ļ�����ʱ����д����ļ������ļ�������ʱ���´���һ���ļ���д��
cat	չʾ�ļ�������
pwd	��ӡ����ǰĿ¼��ַ
mkdir	�����ļ���
mkfs	��ʽ���ļ�ϵͳ
*/
/* Includes ------------------------------------------------------------------*/
#include <fal.h>
#include <dfs_fs.h>
#include <dfs_romfs.h>
#include <fal_cfg.h>
#include <flashdb.h>
/* Private includes ----------------------------------------------------------*/
#include "ulog_file_be.h"
#include "main.h"
//#include "mb_handler.h"
/* ��� DEBUG ͷ�ļ� */
#define DBG_SECTION_NAME               "file"
#define DBG_LEVEL                      DBG_INFO
#include <rtdbg.h>
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define FS_PARTITION_NAME "filesystem"//"W25Q128" //��fal_cfg.h��FAL_PART_TABLE����
#define DB_PARTITION_NAME "flashdb"
#define ENV_VERSION       002          //Ĭ�ϲ����汾
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/* KVDB object */
static struct fdb_kvdb mb_param = { 0 };
/*���д���*/
static uint16_t boot_count = 0;
/*�����־*/
uint16_t flash_save = 0XFF;//0XFF:δ���� 0XBD:�ѱ���
/* default KV nodes ��һ�γ�ʼ��ʱ������Ϣ*/
static struct fdb_default_kv_node default_kv_table[] = 
{
  {"boot_count", &boot_count,           sizeof(boot_count)}, /* int type KV */
  {"save flag",  &flash_save ,          sizeof(flash_save)},
//  {"MBH501"   ,  &usRegHoldingBuf[501] ,sizeof(usRegHoldingBuf[501])},//����ת��������������
//  {"MBH502"   ,  &usRegHoldingBuf[502] ,sizeof(usRegHoldingBuf[502])},//����ת�������ٱ�
//  {"MBH503"   ,  &usRegHoldingBuf[503] ,sizeof(usRegHoldingBuf[503])},//˳ʱ�뷽���Ӧ��ֵ
//  {"MBH504"   ,  &usRegHoldingBuf[504] ,sizeof(usRegHoldingBuf[504])},//�������Ƕ�
//  {"MBH505"   ,  &usRegHoldingBuf[505] ,sizeof(usRegHoldingBuf[505])},//������С�Ƕ�
//  {"MBH506"   ,  &usRegHoldingBuf[506] ,sizeof(usRegHoldingBuf[506])},//��������ٶ�
//  {"MBH507"   ,  &usRegHoldingBuf[507] ,sizeof(usRegHoldingBuf[507])},//������С�ٶ�
//  {"MBH508"   ,  &usRegHoldingBuf[508] ,sizeof(usRegHoldingBuf[508])},//���õ㶯�ٶ�
//  {"MBH509"   ,  &usRegHoldingBuf[509] ,sizeof(usRegHoldingBuf[509])},//����ת������λ�Ƕ�
//  {"MBH510"   ,  &usRegHoldingBuf[510] ,sizeof(usRegHoldingBuf[510])},//��ԭ����
//  {"MBH511"   ,  &usRegHoldingBuf[511] ,sizeof(usRegHoldingBuf[511])},//��ԭ����
//  {"MBH521"   ,  &usRegHoldingBuf[521] ,sizeof(usRegHoldingBuf[521])},//���õ������������
//  {"MBH522"   ,  &usRegHoldingBuf[522] ,sizeof(usRegHoldingBuf[522])},//���õ�����ٱ�
//  {"MBH523"   ,  &usRegHoldingBuf[523] ,sizeof(usRegHoldingBuf[523])},//˳ʱ�뷽���Ӧ��ֵ
//  {"MBH524"   ,  &usRegHoldingBuf[524] ,sizeof(usRegHoldingBuf[524])},//�������
//  {"MBH525"   ,  &usRegHoldingBuf[525] ,sizeof(usRegHoldingBuf[525])},//��С����
//  {"MBH526"   ,  &usRegHoldingBuf[526] ,sizeof(usRegHoldingBuf[526])},//�Ӽ��ٲ���
//  {"MBH527"   ,  &usRegHoldingBuf[527] ,sizeof(usRegHoldingBuf[527])},//�Ӽ�������
//  {"MBH699"   ,  &usRegHoldingBuf[699] ,sizeof(usRegHoldingBuf[699])},//ADC��������
//  {"MBH700"   ,  &usRegHoldingBuf[700] ,sizeof(usRegHoldingBuf[700])},//adc��ֵ
};
/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  VarData_To_Save
  * @param  None.
  * @retval None.
  * @note   ��ȡ�����������⣬���ȳ������±��������ڶ�������
*/
void MB_Param_Save(void)
{
  struct fdb_blob blob;
  const uint16_t *temp;
  flash_save = 0XBD;
  for(uint8_t i = 0; i < sizeof(default_kv_table) / sizeof(struct fdb_default_kv_node);i++)
  {
    temp = (const uint16_t *)default_kv_table[i].value;
    //���� blob ����
    fdb_kv_set_blob(&mb_param,default_kv_table[i].key, fdb_blob_make(&blob,temp,default_kv_table[i].value_len));
  }
}
/**
  * @brief  VarData_To_Read
  * @param  None.
  * @retval None.
  * @note   ��ȡ�����������⣬���ȳ������±��������ڶ�������
*/
void MB_Param_Read(void)
{
  struct fdb_blob blob;
  const uint16_t *temp;
  for(uint8_t i = 0; i < sizeof(default_kv_table) / sizeof(struct fdb_default_kv_node);i++)
  {
    temp = (const uint16_t *)default_kv_table[i].value;
    //���� blob ����
    fdb_kv_get_blob(&mb_param,default_kv_table[i].key, fdb_blob_make(&blob,temp,default_kv_table[i].value_len));
  }
}
static void kvdb_basic_sample(fdb_kvdb_t kvdb)
{
    struct fdb_blob blob;
    int boot_count = 0;

    { /* GET the KV value */
        /* get the "boot_count" KV value */
        fdb_kv_get_blob(kvdb, "boot_count", fdb_blob_make(&blob, &boot_count, sizeof(boot_count)));
        /* the blob.saved.len is more than 0 when get the value successful */
        if (blob.saved.len > 0) {
            LOG_D("get the 'boot_count' value is %d", boot_count);
        } else {
            LOG_W("get the 'boot_count' failed");
        }
    }

    { /* CHANGE the KV value */
        /* increase the boot count */
        boot_count ++;
        /* change the "boot_count" KV's value */
        fdb_kv_set_blob(kvdb, "boot_count", fdb_blob_make(&blob, &boot_count, sizeof(boot_count)));
        LOG_I("Burn the number :%d", boot_count);
    }
}
/**
  * @brief  
  * @param  None
  * @retval None
  * @note   
*/
int Flash_KVDB_Init(void)
{
  fdb_err_t result;
  struct fdb_default_kv default_kv;
  default_kv.kvs = default_kv_table;
  default_kv.num = sizeof(default_kv_table) / sizeof(default_kv_table[0]);
  mb_param.ver_num = ENV_VERSION;//ÿ�γ�ʼ�����汾�ţ��Զ����°汾
  
  /*��ʼ�� KVDB
    ����	      ����
    db	        ���ݿ����
    name	      ���ݿ�����
    path	      FAL ģʽ���������еķ��������ļ�ģʽ�����ݿⱣ���·��
    default_kv	Ĭ�� KV ���ϣ���һ�γ�ʼ��ʱ�������Ĭ�� KV д�����ݿ���
    user_data	  �û��Զ������ݣ�û��ʱ���� NULL
    ����	      ������
  */
  result = fdb_kvdb_init(&mb_param, "env",DB_PARTITION_NAME, &default_kv, NULL);
  if (result != FDB_NO_ERR)   
  {
   return RT_ERROR;
  }
  kvdb_basic_sample(&mb_param);
  
  MB_Param_Read();
//  //FLASH��û������
//  if(flash_save != 0XBD)
//  {
//    DEFAULT_DATA_SET;
//    SAVE_SET;
//  }
  return RT_EOK;
}
/**
  * @brief  �ļ�ϵͳ��ʼ��.
  * @param  None.
  * @retval None.
  * @note   None.
*/
static int FileSystem_Init(void)
{
  /* ��ʼ�� fal */
  fal_init();
  //�����ȴ���rom�ļ�ϵͳ�����ṩ����Ĺ���Ŀ¼
  if(dfs_mount(RT_NULL,"/","rom",0,&(romfs_root)) == 0)
  {
      LOG_I("ROM file system initializated!");
      /* ���� mtd �豸 */
      struct rt_device *mtd_dev = fal_mtd_nor_device_create(FS_PARTITION_NAME);
      if (!mtd_dev)
      {
          LOG_E("Can't create a mtd device on '%s' partition.", FS_PARTITION_NAME);
      }
      else
      {
        /* ���� littlefs  */
        if (dfs_mount(FS_PARTITION_NAME, "/flash", "lfs", 0, 0) == 0)
        {
            LOG_I("Filesystem /flash initialized!");
        }
        else
        {
            /* ��ʽ���ļ�ϵͳ */
            dfs_mkfs("lfs", FS_PARTITION_NAME);
            /* ���� lfs */
            if (dfs_mount(FS_PARTITION_NAME, "/flash", "lfs", 0, 0) == 0)
            {
                LOG_I("Filesystem /flash initialized!");
            }
            else
            {
                LOG_E("Failed to initialize filesystem /flash!");
            }
        }
      }
  }
  else
  {
      LOG_E("ROM file system initializate failed!");
  }
  #if(OUT_FILE_ENABLE == 1)
  extern void sys_log_file_backend_init(void);
  sys_log_file_backend_init();
  extern void motion_log_file_backend_init(void);
  motion_log_file_backend_init();
  #endif
  /*���ݿ��ʼ��*/
  Flash_KVDB_Init();
  return RT_EOK;
}
INIT_ENV_EXPORT(FileSystem_Init);