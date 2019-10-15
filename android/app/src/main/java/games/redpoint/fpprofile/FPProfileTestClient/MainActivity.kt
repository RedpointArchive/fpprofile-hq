package games.redpoint.fpprofile.FPProfileTestClient

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import kotlinx.android.synthetic.main.activity_main.*
import java.util.logging.LogManager
import kotlin.concurrent.thread
import kotlin.system.exitProcess

class MainActivity : AppCompatActivity(), Logger {

    override fun onLog(message: String) {
        sample_text.text = sample_text.text.toString() + "\n" + message
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContentView(R.layout.activity_main)
        this.onLog("Starting fpprofile test...")

        thread(start = true) {
            var result = this.fpprofileStart( "103.31.113.109:40000")
            if (!result) {
                runOnUiThread(java.lang.Runnable {
                    this.onLog("fpprofile test could not start")
                })
            } else {
                while (this.fpprofileStep()) {
                    // wait
                }

                var end = this.fpprofileEnd()

                runOnUiThread(java.lang.Runnable {
                    this.onLog("fpprofile test compete, with exit code " + end)
                })
            }
        }
    }

    external fun fpprofileStart(clientConnectTo: String): Boolean
    external fun fpprofileStep(): Boolean
    external fun fpprofileEnd(): Int

    companion object {

        // Used to load the 'native-lib' library on application startup.
        init {
            System.loadLibrary("fpprofile")
        }
    }
}
