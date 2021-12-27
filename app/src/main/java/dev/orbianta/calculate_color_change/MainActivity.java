package dev.orbianta.calculate_color_change;

import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Bitmap;
import android.os.Build;
import android.os.Bundle;
import android.os.PowerManager;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;

import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.JavaCameraView;
import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.Mat;

import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

import dev.orbianta.calculate_color_change.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity
{

    private DateTimeFormatter dtf;

    private PowerManager pm;
    public PowerManager.WakeLock pm_wakelock;

    private Button exec_b;
    private JavaCameraView ocv_camera;
    private EditText uilog;
    private ImageView camera_surface;

    public Boolean is_process = false;

    private ActivityMainBinding binding;

    @RequiresApi(api = Build.VERSION_CODES.O)
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        exec_b = findViewById(R.id.run_btn);
        ocv_camera = findViewById(R.id.ocv_camera);
        uilog = findViewById(R.id.uilog);
        camera_surface = findViewById(R.id.camera_surface);


        pm = (PowerManager) getSystemService(POWER_SERVICE);
        pm_wakelock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, "wakelock::CCC");

        uilog.append("CCC app by 0rbianta\n");

        System.loadLibrary("app");
        uilog.append("Linking native side: Ok\n");

        if (!(Build.VERSION.SDK_INT >= Build.VERSION_CODES.O))
        {
            Log.d("Error", "System SDK is lower then 26(Android O)");
            System.exit(1);
        }

        dtf = DateTimeFormatter.ofPattern("yyyy/MM/dd HH:mm:ss");




        exec_b.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View view)
            {
                if (is_process)
                {
                    uilog.append("Stopping Color Processing...\n");

                    pm_wakelock.release();
                    uilog.append("Disable wakelock: Ok\n");

                    close_log();
                    uilog.append("Close log file: Ok\n");
                    exec_b.setText("Start");

                }
                else
                {
                    uilog.append("Starting Color Processing...\n");

                    pm_wakelock.acquire();
                    uilog.append("Enable wakelock: Ok\n");

                    if (!check_external_fs_access())
                    {
                        Log.d("Error", "Failed to get R/W access on external storage.");
                        System.exit(1);
                    }
                    else
                        uilog.append("Get R/W access for external storage: Ok\n");

                    gen_new_log();
                    uilog.append("Generate new logging file: Ok\n");

                    exec_b.setText("Stop");
                }
                is_process = !is_process;
            }
        });



    }

    @Override
    protected void onResume()
    {
        super.onResume();
        if (!OpenCVLoader.initDebug())
        {
            Log.d("Error","OpenCV Java side failed to load.");
            System.exit(1);

        }
        else
        {
            uilog.append("Load OpenCV Java side: Ok\n");
            frame_process();
        }

    }



    private void frame_process()
    {
        ocv_camera.enableView();

        ocv_camera.setCvCameraViewListener(new CameraBridgeViewBase.CvCameraViewListener()
        {
            //////////////////////

            @Override
            public void onCameraViewStarted(int width, int height)
            {

            }

            @Override
            public void onCameraViewStopped()
            {

            }

            @RequiresApi(api = Build.VERSION_CODES.O)
            @Override
            public Mat onCameraFrame(Mat inputFrame)
            {

                Mat color_out = inputFrame.clone();
                if (is_process)
                {
                    int result = proc_color(color_out.getNativeObjAddr());
                    if (result != -2)
                        writeln_to_log(dtf.format(LocalDateTime.now()) + " => " + result);
                }


                runOnUiThread(new Runnable()
                {
                    @Override
                    public void run()
                    {

                        Bitmap m2b = Bitmap.createBitmap(color_out.cols(), color_out.rows(), Bitmap.Config.RGB_565);
                        Utils.matToBitmap(color_out, m2b);
                        camera_surface.setImageBitmap(m2b);

                    }
                });

                return color_out;
            }

            //////////////////////
        });

    }

    public native void gen_new_log();
    public native void writeln_to_log(String ln);
    public native boolean check_external_fs_access();
    public native void close_log();
    public native int proc_color(long out);

}