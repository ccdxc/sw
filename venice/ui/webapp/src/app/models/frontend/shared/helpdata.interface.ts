
import { ComponentPortal, TemplatePortal } from '@angular/cdk/portal';

export interface HelpData {
  component?: ComponentPortal<any>;
  template?: TemplatePortal;
  // Id to fetch generated help for
  id?: string;
}
