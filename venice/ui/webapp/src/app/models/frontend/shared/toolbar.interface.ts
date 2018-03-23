export interface BreadcrumbItem {
    label: string;
    url: string;
  }

  export interface ToolbarButton {
    text: string;
    cssClass: string;
    callback: (...args: any[]) => void;
  }

  export interface ToolbarData {
    buttons: ToolbarButton[];
    breadcrumb: BreadcrumbItem[];
  }
