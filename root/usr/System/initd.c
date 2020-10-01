# include <status.h>

# include <kernel/kernel.h>
# include <kernel/access.h>
# include <kernel/rsrc.h>
# include <kernel/tls.h>

# include <system_init.h>
# include <system_log.h>

inherit access API_ACCESS;
inherit rsrc API_RSRC;
inherit tls API_TLS;

# define DUMP_INTERVAL  7200 /* every two hours */

int memory_high, memory_max;

int statedump_offset;
int dump;

static void create() {
   access::create();
   rsrc::create();
   tls::create();

   add_user("System");
   set_access("System", "/", FULL_ACCESS);
   /* allow everyone to read us (especially ~/include/std.h) */
   set_global_access("System", TRUE);

   call_out("memory_watcher", 1, FALSE);

   dump = call_out("perform_statedump",
           DUMP_INTERVAL - (time() % DUMP_INTERVAL) +
           statedump_offset);

}

private void load_settings() {
    string before, after, text;

    memory_high = 0;
    memory_max = 0;
    statedump_offset = 0;
    if (text = read_file(SETTINGS_FILE)) {
        /* remove all # lines */
        while (sscanf(text, "%s#%*s\n%s", before, after)) {
            text = before + after;
        }

        sscanf(text, "%*smemory_high %d\n", memory_high);
        sscanf(text, "%*smemory_max %d\n",  memory_max);
        sscanf(text, "%*sstatedump_offset %d\n", statedump_offset);
    }
    error("missing file " + SETTINGS_FILE);
}

int query_memory_high() {
   return memory_high ? memory_high : 128;
}

int query_memory_max() {
   return memory_max ? memory_max : 256;
}

void shutdown() {
   ::shutdown();
}

# define MEGABYTE (1024 * 1024)
static void
memory_watcher(int high, varargs int last_used, int last_allocated)
{
    int used, allocated;

    used      = status()[ST_SMEMUSED] + status()[ST_DMEMUSED];
    allocated = status()[ST_SMEMSIZE] + status()[ST_DMEMSIZE];

    if (allocated > query_memory_max() * MEGABYTE) {
        DEBUG("memory_watcher: Max allocation triggered swapout: " +
            ((used + MEGABYTE - 1) / MEGABYTE) + " MB in use; " + ((allocated + MEGABYTE - 1) / MEGABYTE) + " MB allocated");
        high = FALSE;
        swapout();
    } else {
        if (!high && allocated > query_memory_high() * MEGABYTE) {
            high = TRUE;
        }
        if (high && used / 2 < allocated / 3) {
            DEBUG("memory_watcher: Triggered swapout: " +
                ((used + MEGABYTE - 1) / MEGABYTE) + " MB in use; " + ((allocated + MEGABYTE - 1) / MEGABYTE) + " MB allocated");
            high = FALSE;
            swapout();
        }
   }
   call_out("memory_watcher", 1, high, used, allocated);
}
