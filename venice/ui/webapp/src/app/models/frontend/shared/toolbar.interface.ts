export interface BreadcrumbItem {
  label: string;
  url?: string;
  command?: (...args: any[]) => void;
  payload?: any;
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
