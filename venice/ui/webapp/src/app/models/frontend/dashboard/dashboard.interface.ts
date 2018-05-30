// This object doesn't need an id, as the pinned payload should already have the module and component name,
// and will not use the id
export interface PinnedDashboardWidgetData {
  // Space to occupy in the gridster
  rows: number;
  cols: number;
}

export interface DashboardWidgetData extends PinnedDashboardWidgetData {
  // Start position of widget in the gridster
  x?: number;
  y?: number;
  // Used by gridster to know which widget to create
  // Should be the same as the widget name
  id: string;
  // Will be set to true once the containing gridster is ready.
  dsbdGridsterItemIsReady: boolean;
}

export interface DashboardWidget {
  /* Used to notify widget that it is being placed on the dashboard
    * Important for pinned widgets
    */
  dashboardSetup?();

  /* Returns the settings to be used for placing the widget
    * on the gridster.
    */
  getDashboardSettings?(): DashboardWidgetData;
}
