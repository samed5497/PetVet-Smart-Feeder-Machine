#include <core/chooser.h>

#ifdef PetVet

#include <functions/tasks.h>
void loop() // loop() işlevi boş bırakıldı
{
    vTaskDelay(60000 / portTICK_PERIOD_MS); // 1 Hz
}
#endif
