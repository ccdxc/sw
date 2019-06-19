export interface UserPreference {
  telemetry: TelemetryPref;
}

export interface TelemetryPref {
    // Telemetry page last saved settings?:
    lastSelectedTimeRange?: any;
    items: TelemetryItem[];
    // Mapping from a graph's id to its configuration
    configuredGraphs: { [id: string]: GraphConfig};

}

export enum ChartItemType {
  folder = 'folder',
  chart = 'chart',
}

export interface GraphTransformConfig {
  transforms: { [transformName: string ]: any };
}

export interface DataTransformConfig {
  transforms: { [transformName: string ]: any };
  measurement: string;
  fields: string[];
}

export interface GraphConfig {
  id: string;
  graphTransforms: GraphTransformConfig;
  dataTransforms: DataTransformConfig[];
}

// We call load on each data transform with data passed in.
// For Each transform, we create them by transform name.
// then we call the load method with the config passed in.

// For saving. we call the save method on each  data transform, and append
// each save call on a transfrom emits a config, which can be passed in to recreate.

export interface FolderConfig {
  name: string;
  coverGraph?: string;
  items: TelemetryItem[];
}

export interface TelemetryItem {
  type: ChartItemType;
  folderConfig?: FolderConfig;
  graphID?: string;
}
