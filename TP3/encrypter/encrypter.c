/** @file
		@brief encrypter
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

static int 
device_open (struct inode *, struct file *);

static int 
device_release (struct inode *, struct file *);

static ssize_t 
device_read (struct file *, char *, size_t, loff_t *);

static ssize_t 
device_write (struct file *, const char *, size_t, loff_t *);

static int __init init_encrypter (void);

static void __exit cleanup_encrypter (void);

module_init (init_encrypter);
module_exit (cleanup_encrypter);

#define SUCCESS 0
#define DEVICE_NAME "encrypter"	
#define BUF_LEN 80		

static int major;		
static char msg [BUF_LEN];
static unsigned long msg_buf_len = 0;

static struct file_operations fops = 
{
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

/** Esta función es llamada cuando el módulo es cargado
*/
static int __init init_encrypter (void)
{
	int reg;

  reg = register_chrdev (0, DEVICE_NAME, &fops);

  if (~IS_ERR_VALUE (reg))
	  {
	      printk (DEVICE_NAME ": Major = %d.\n", reg);
	      
	      major = reg;

	      return 0;
	  }

	return 0;
}

/** Esta función es llamada cuando el módulo es descargado
*/
static void __exit cleanup_encrypter (void)
{
	unregister_chrdev (major, DEVICE_NAME);
}

/** Esta función es llamada cuando un proceso intenta abrir el archivo del dispositivo
*/
static int 
device_open (struct inode *inode, struct file *file)
{
	try_module_get (THIS_MODULE);

	return SUCCESS;
}

/** Esta función es llamada cuando un proceso cierra el archivo del dispositivo
*/
static int 
device_release (struct inode *inode, struct file *file)
{
	module_put (THIS_MODULE);

	return 0;
}

/** Esta función es llamada cuando un proceso que ya abrió el archivo del dispositivo, 
		intenta leer de él. Muestra la cadena de caracteres encriptada
*/
static ssize_t 
device_read (struct file *file, char __user *buff, size_t len, loff_t *off)
{
	int bytes_read;

	copy_to_user (buff, msg, msg_buf_len);
	bytes_read = msg_buf_len;
	
	if (*off == 0) 
		{
			*off += 1;

			return bytes_read;
		} 
	else 
		{
			return 0;
		}
}

/** Esta función es llamada cuando un proceso escribe en el archivo del dispositivo.
		Encripta la cadena de caracteres ingresada. 
*/
static ssize_t
device_write (struct file *filp, const char *buff, size_t len, loff_t * off)
{
	int i;
	char aux [len];

	msg_buf_len = len;
	
	if (msg_buf_len > BUF_LEN) 
		{
			msg_buf_len = BUF_LEN;
		}
	
	if (copy_from_user (aux, buff, msg_buf_len))
		{
			return -EFAULT;
		}

	for (i = 0; i < msg_buf_len; i++)
		{
			aux [i] = aux [i] + 3;
		}

	memcpy (msg, aux, msg_buf_len);
	
	return msg_buf_len;
}