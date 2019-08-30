# Metrics page design doc

## Overview
Metrics are reported by Venice and DSCs to Citadel. Inside Citadel, data is stored inside Influx databases (one per tenant). An influx database consists of a measurement (Table name) and fields/tags (columns of the table). Influx querying is optimized on tags, but externally we make no differentiation between fields and tags, we consider them all fields. The reporting system for metrics is built such that Citadel doesn't need any prior knowledge of what data it will be sent. In order for the UI to be aware of the data available, metric annotaions have been added.

### Generating the metric metadata

#### Backend metric files
Metrics reported from agents running on the DSCs are written in proto files (ftestats.proto). These will be read in my a code generator to create the golang code to report these metrics to Venice. A different code generator will also read these files and generate a metric metadata JSON files that contains the metric annotations in the proto. These files live under `sw/metrics/**/**.json`. 

#### Generating venice-sdk/metrics/generated/metadata.ts
`venice-sdk/generators/metrics.ts` reads all of the metric JSON files, and combines them with the metric annotations in `custom-metrics.js` to produce `metadata.ts`. `custom-metrics.js` are handwritten metadata file for the Venice metrics, since these do not come from protos currently.

### Telemetry components
There are 3 components that make up the metrics page.
1. TelemetryChart - Responsible for creating the metrics query, and graphing the response.
2. TelemetryChartEdit - Responsible for allowing users to configure the graph.
3. TelemetryChartView - Responsible for displaying TelemetryChart on the landing page.

#### Design goals
The focus of this design is to isolate features as much as possible, and provide a structured way in which new features can be added. The result is that there are many files, but each feature is usually limited to its own file. Adding features to telemetrychart directly should be avoided as much as possible. Instead, it is better to add a new hook for other transforms to use.

#### Datasource 
A graph contains one or more data sources. Data sources are made up of a single measurement, one or more fields, and a set of query/display options. It is one to one with the number of queries we issue to the backend. 

#### Metric Transform
A datasource contains a set of metric transforms. A transform allows for mutating the query and/or mutating the resulting dataset at different stages.
When creating a query, TelemetryChart will call the appropriate hook methods on each transform. Transforms also have methods to indicate that a new query needs to be issued (`requestMetrics`). See transforms/types for definition and hooks. While transforms should be ideally independent, it is possible for a transform to access the state of another transform.

#### Graph Transforms
Graph transforms are for mutating graph wide options, and are mainly for setting the chart.js graph options.

#### Sample Flow
1. User adds a filter (filter transform html inside telemetry-edit component)
2. Filter transforms detects the user has added input, and calls requestMetrics()
3. TelemetryChart is listening to the subscription, and calls getMetrics.
4. TelemetryChart creates a blank query for each datasource, and calls `transformQuery` for each transform.
5. TelemetryChart takes the resulting query and sends it to Venice. Venice responds with a metric response
6. The metric response is converted into a dataset. Each transform's `transformDataset` function is called, and they can modify the dataset options/data.
7. Graph Transforms are called to modify the graph options.
8. The datasets and the graph options are passed to chart.js