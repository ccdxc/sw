import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import {
  ChartModule, SplitButtonModule, StepsModule, AccordionModule, CalendarModule, PanelModule, DataTableModule, SharedModule,
  BlockUIModule, DropdownModule, SelectItem, SliderModule,
  ToolbarModule, FieldsetModule, InputMaskModule, InputSwitchModule, InputTextareaModule, CheckboxModule,
  MultiSelectModule, ListboxModule, RadioButtonModule, DataGridModule, GrowlModule, Message, OverlayPanelModule,
  SelectButtonModule, TooltipModule, AutoCompleteModule
} from 'primeng/primeng';

/**
 * This module control what primeNG widgets to use in our application.
 * We specify import/export here to avoid overloading app.module.ts.
 * In case, we don't use mprimeNG, we just simple take a small change in app.module.ts
 */
@NgModule({
  imports: [
    ChartModule, SplitButtonModule, StepsModule, AccordionModule, CalendarModule, PanelModule, DataTableModule, SharedModule,
    BlockUIModule, DropdownModule,  SliderModule,
    ToolbarModule, FieldsetModule, InputMaskModule, InputSwitchModule, InputTextareaModule, CheckboxModule,
    MultiSelectModule, ListboxModule, RadioButtonModule, DataGridModule, GrowlModule, OverlayPanelModule,
    SelectButtonModule, TooltipModule, AutoCompleteModule
  ],
  exports: [
    ChartModule, SplitButtonModule, StepsModule, AccordionModule, CalendarModule, PanelModule, DataTableModule, SharedModule,
    BlockUIModule, DropdownModule,  SliderModule,
    ToolbarModule, FieldsetModule, InputMaskModule, InputSwitchModule, InputTextareaModule, CheckboxModule,
    MultiSelectModule, ListboxModule, RadioButtonModule, DataGridModule, GrowlModule, OverlayPanelModule,
    SelectButtonModule, TooltipModule, AutoCompleteModule
  ],
  declarations: []
})
export class PrimengModule { }
