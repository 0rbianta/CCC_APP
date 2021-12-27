// C++
#include <jni.h>
#include <string>
#include <opencv2/opencv.hpp>
#include <vector>

// C
#include <time.h>
#include <unistd.h>

// Use required namespaces
using namespace std;
using namespace cv;


// Define size of process frame
#define process_size Size(720, 720)
// Define color space of processing
#define process_color COLOR_RGBA2RGB
// Define color space of output
#define output_color COLOR_RGBA2GRAY
// Define log files save chdir
#define log_save_path "/sdcard"
// Define buffer size of log_file_name
#define fname_buf_size 250


// Define last5 vector
vector<int> last5;
// Define log file
FILE *log_file = NULL;



extern "C"
JNIEXPORT jint JNICALL
Java_dev_orbianta_calculate_1color_1change_MainActivity_proc_1color(JNIEnv *env, jobject thiz, jlong out)
{
    // Create an return value
    int outbuf = -2;
    // Get duplicate of frame
    Mat frame = *(Mat *)out;
    // Convert frame color to color defined by process_color
    cvtColor(frame, frame, process_color);
    // Resize frame to size defined by process_size
    resize(frame, frame, process_size);

    // Check if last5 vector contains values more then or equal to 5
    if (last5.size() >= 5)
    {
        // Create a loop cursor
        int u = -1;
        // Create a sum buffer
        int sum = 0;
        // Loop through each index of the last5 vector
        while (u++, u < last5.size())
            // Sum values in the last5
            sum += last5[u];

        // Calculate arithmetic mean of sum
        outbuf = sum / (u + 1);

        // Erase first value in the last5 vector
        last5.erase(last5.begin());
    }

    int fbuf = 0;
    // Loop through columns and rows of the frame
    for (unsigned int x = 0; x < frame.rows; x++)
        for (unsigned int y = 0; y < frame.cols; y++)
        {
            // Get the pixel at the (x, y) position
            Vec3b pixel = frame.at<Vec3b>(x, y);
            // Get each pixels values
            uchar r = pixel[0];
            uchar g = pixel[1];
            uchar b = pixel[2];
            // Calculate arithmetic mean of them
            int pxm = (r + g + b) / 3;
            // Add data to pixel buffer
            fbuf += pxm;
        }

    // Add new pixels information to the last5 vector
    last5.push_back(fbuf);


    // Get original frame and overwrite on it
    Mat &o = *(Mat *)out;
    // Convert original frames color to color defined by output_color
    cvtColor(o, o, output_color);

    return outbuf;
}


extern "C"
JNIEXPORT void JNICALL
Java_dev_orbianta_calculate_1color_1change_MainActivity_gen_1new_1log(JNIEnv *env, jobject thiz)
{
    // Get raw time
    time_t tms = time(NULL);
    // Get time information with a struct data
    tm *t = localtime(&tms);

    // Allocate space for file name string
    char *fpath = (char *)malloc(sizeof(char) * fname_buf_size);

    // Create the log file path with its name
    snprintf(fpath, fname_buf_size, "%s/CCC_LOG_%d_%d_%d_%d_%d.log", log_save_path, t->tm_mday,
             t->tm_mon, t->tm_year + 1900, t->tm_hour, t->tm_min);

    // Check if log file previously opened
    if (log_file != NULL)
        // Close log file
        fclose(log_file);


    // Open and create a new log file
    log_file = fopen(fpath, "w+");
    // Check opening file successful or not
    if (log_file == NULL)
    {
        // If opening file failed, write error message to stderr
        fprintf(stderr, "Failed to create a new log file. Sending SIGKILL to the program with pid %d", getpid());
        // Kill the process
        kill(getpid(), SIGKILL);
    }
    else
        // Add a header to log file
        fprintf(log_file, "***** CCC LOGGING by 0rbianta *****\n");

    // Free up the string buffer
    free(fpath);

}


extern "C"
JNIEXPORT void JNICALL
Java_dev_orbianta_calculate_1color_1change_MainActivity_writeln_1to_1log(JNIEnv *env, jobject thiz, jstring log)
{

    // Convert Java string to C++ string
    string cpps = env->GetStringUTFChars(log, 0);
    // Convert C++ string to C string
    const char *log_ln = cpps.c_str();

    // Write C string to log file
    fprintf(log_file, "%s\n", log_ln);

}


extern "C"
JNIEXPORT jboolean JNICALL
Java_dev_orbianta_calculate_1color_1change_MainActivity_check_1external_1fs_1access(JNIEnv *env, jobject thiz)
{

    // Check R/W access for external storage
    int wp = access(log_save_path, W_OK) == 0;
    int rp = access(log_save_path, R_OK) == 0;


    return wp && rp;
}


extern "C"
JNIEXPORT void JNICALL
Java_dev_orbianta_calculate_1color_1change_MainActivity_close_1log(JNIEnv *env, jobject thiz)
{
    // Close log file
    if (log_file != NULL)
        fclose(log_file);
}