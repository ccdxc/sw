import { MenuItem, SelectItem } from 'primeng/primeng';

export interface BreadcrumbItem {
  label: string;
  url?: string;
  command?: (...args: any[]) => void;
  payload?: any;
}

export interface ToolbarButton {
  text: string;
  cssClass?: string;
  matTooltipClass?: string;
  callback: (...args: any[]) => void;
  computeClass?: any;
  genTooltip?: () => string;
}

export interface SplitButton {
  text: string;
  callback: (...args: any[]) => void;
  items: MenuItem[];  // see splitbutton API.
  icon?: string;
  styleClass?: string; // splitbutton style-class
  menuStyleClass?: string;  // splitbutton menu style-class
}

export interface Dropdown {
  callback: (...args: any[]) => void;
  options: SelectItem[];  // see Dropdown API.
  model: any; // model
  styleClass?: string;     // Dropdown style-class
  panelStyleClass?: string; // Dropdown-panel style-class
  placeholder?: string;
  showClear?: boolean;
  group?: boolean;
  filter?: boolean;
  editable?: boolean;
  optionLabel?: string;
}

export interface ToolbarData {
  buttons?: ToolbarButton[];
  breadcrumb?: BreadcrumbItem[];
  splitbuttons?: SplitButton[];
  dropdowns?: Dropdown[];
}
