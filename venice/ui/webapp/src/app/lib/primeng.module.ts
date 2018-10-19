import { NgModule } from '@angular/core';
import { AccordionModule, AutoCompleteModule, BlockUIModule, BreadcrumbModule, CalendarModule, ChartModule, CheckboxModule, DataGridModule, DataTableModule, DropdownModule, FieldsetModule, GrowlModule, InputMaskModule, InputSwitchModule, InputTextareaModule, ListboxModule, MultiSelectModule, OverlayPanelModule, PanelModule, RadioButtonModule, SelectButtonModule, SharedModule, SliderModule, SplitButtonModule, StepsModule, ToolbarModule, TooltipModule, InputTextModule } from 'primeng/primeng';
import { TableModule } from 'primeng/table';
import { ToastModule } from 'primeng/toast';


/**
 * This module control what primeNG widgets to use in our application.
 * We specify import/export here to avoid overloading app.module.ts.
 * In case, we don't use mprimeNG, we just simple take a small change in app.module.ts
 */
@NgModule({
  imports: [
    ChartModule, SplitButtonModule, StepsModule, AccordionModule, CalendarModule, PanelModule, DataTableModule, SharedModule,
    BlockUIModule, DropdownModule, SliderModule,
    ToolbarModule, FieldsetModule, InputTextModule, InputMaskModule, InputSwitchModule, InputTextareaModule, CheckboxModule,
    MultiSelectModule, ListboxModule, RadioButtonModule, DataGridModule, GrowlModule, OverlayPanelModule,
    SelectButtonModule, TooltipModule, AutoCompleteModule, BreadcrumbModule, TableModule, InputTextModule, ToastModule
  ],
  exports: [
    ChartModule, SplitButtonModule, StepsModule, AccordionModule, CalendarModule, PanelModule, DataTableModule, SharedModule,
    BlockUIModule, DropdownModule, SliderModule,
    ToolbarModule, FieldsetModule, InputTextModule, InputMaskModule, InputSwitchModule, InputTextareaModule, CheckboxModule,
    MultiSelectModule, ListboxModule, RadioButtonModule, DataGridModule, GrowlModule, OverlayPanelModule,
    SelectButtonModule, TooltipModule, AutoCompleteModule, BreadcrumbModule, TableModule, InputTextModule, ToastModule
  ],
  declarations: []
})
export class PrimengModule { }
