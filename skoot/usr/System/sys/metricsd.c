# include <System.h>
# include <System/log.h>

private mapping metrics;

static void
create() {
    metrics = ([ ]);
}

void
increment(string met, varargs int amount)
{
    string source;

    source = previous_program();

    if(!metrics) {
        metrics = ([ ]);
    }
    if(!metrics[source]) {
        metrics[source] = ([ ]);
    }
    if(!metrics[source][met]) {
        metrics[source][met] = 0;
    }
    if(!amount) {
        amount = 1;
    }
    metrics[source][met] += amount;
}

mapping
query_metrics_for(string progname)
{
    return metrics[progname][..];
}

string*
query_sources(void)
{
    return map_indices(metrics);
}
