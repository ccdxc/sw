import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { PagebodyComponent } from './pagebody/pagebody.component';
import { FlexLayoutModule } from '@angular/flex-layout';
import { SpinnerComponent } from './spinner/spinner.component';
import { DsbdwidgetheaderComponent } from './dsbdwidgetheader/dsbdwidgetheader.component';
import { ModalheaderComponent } from './modal/modalheader/modalheader.component';
import { ModalbodyComponent } from './modal/modalbody/modalbody.component';
import { ModalitemComponent } from './modal/modalitem/modalitem.component';
import { ModalcontentComponent } from './modal/modalcontent/modalcontent.component';
import { ModalwidgetComponent } from './modal/modalwidget/modalwidget.component';

import { MaterialdesignModule } from '@lib/materialdesign.module';
import { TableheaderComponent } from '@app/components/shared/tableheader/tableheader.component';
import { LazyrenderComponent } from './lazyrender/lazyrender.component';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';
import { SafeStylePipe } from '@app/components/shared/Pipes/SafeStyle.pipe';
import { AlertseventsComponent } from '@app/components/shared/alertsevents/alertsevents.component';
import { PrimengModule } from '@app/lib/primeng.module';
import { FormsModule, ReactiveFormsModule } from '@angular/forms';
import { WhitespaceTrimDirective } from '@app/components/shared/directives/whitespacetrim.directive';
import { FieldselectorComponent } from './fieldselector/fieldselector.component';
import { HerocardComponent } from './herocard/herocard.component';
import { WidgetsModule } from 'web-app-framework';
import { SorticonComponent } from './sorticon/sorticon.component';
import { BasecardComponent } from './basecard/basecard.component';
import { LinegraphComponent } from './linegraph/linegraph.component';
import { FlipComponent } from './flip/flip.component';
import { ErrorTooltipDirective } from './directives/errorTooltip.directive';
import { TablevieweditHTMLComponent } from './tableviewedit/tableviewedit.component';
import { RoleGuardDirective } from './directives/roleGuard.directive';
import { LabeleditorComponent } from './labeleditor/labeleditor.component';
import { AdvancedSearchComponent } from './advanced-search/advanced-search.component';
import { TimeRangeComponent } from './timerange/timerange.component';
import { UIChartComponent } from './primeng-chart/chart';
import { OrderedlistComponent } from './orderedlist/orderedlist.component';
import { DragDropModule } from '@angular/cdk/drag-drop';
import { ChipsComponent } from './chips/chips.component';
import { FormChipsComponent } from './chips/formChips.component';
import { DomHandler } from 'primeng/api';
import { FeatureGuardDirective } from './directives/featureGuard.directive';
import { AlertstableComponent } from './alertsevents/alertstable/alertstable.component';
import { EventstableComponent } from './alertsevents/eventstable/eventstable.component';
import { SyslogComponent } from './syslog/syslog.component';
import { TelemetrychartComponent } from './telemetry/telemetrychart/telemetrychart.component';
import { TelemetrycharteditComponent } from './telemetry/telemetrychart-edit/telemetrychartedit.component';
import { TelemetrychartviewComponent } from './telemetry/telemetrychart-view/telemetrychartview.component';
import {AlertIndicationBarComponent} from './alert-indication-bar/alert-indication-bar.component';
import { DscprofilesetterComponent } from './dscprofilesetter/dscprofilesetter.component';
import { ExportLogsComponent } from './exportlogs/exportlogs.component';

@NgModule({
  imports: [
    CommonModule,
    FlexLayoutModule,
    MaterialdesignModule,
    PrimengModule,
    FormsModule,
    ReactiveFormsModule,
    FormsModule,
    WidgetsModule,
    DragDropModule
  ],
  declarations: [PagebodyComponent,
    SpinnerComponent,
    DsbdwidgetheaderComponent,
    ModalheaderComponent,
    ModalbodyComponent,
    ModalitemComponent,
    ModalcontentComponent,
    ModalwidgetComponent,
    TableheaderComponent,
    LazyrenderComponent,
    PrettyDatePipe,
    SafeStylePipe,
    AlertseventsComponent,
    WhitespaceTrimDirective,
    FieldselectorComponent,
    HerocardComponent,
    SorticonComponent,
    BasecardComponent,
    LinegraphComponent,
    FlipComponent,
    ErrorTooltipDirective,
    TablevieweditHTMLComponent,
    RoleGuardDirective,
    FeatureGuardDirective,
    LabeleditorComponent,
    AdvancedSearchComponent,
    TimeRangeComponent,
    UIChartComponent,
    OrderedlistComponent,
    ChipsComponent,
    FormChipsComponent,
    AlertstableComponent,
    EventstableComponent,
    SyslogComponent,
    TelemetrychartComponent,
    TelemetrycharteditComponent,
    TelemetrychartviewComponent,
    AlertIndicationBarComponent,
    DscprofilesetterComponent,
    ExportLogsComponent
  ],
  exports: [
    PagebodyComponent,
    SpinnerComponent,
    DsbdwidgetheaderComponent,
    ModalheaderComponent,
    ModalbodyComponent,
    ModalitemComponent,
    ModalcontentComponent,
    ModalwidgetComponent,
    TableheaderComponent,
    LazyrenderComponent,
    PrettyDatePipe,
    SafeStylePipe,
    AlertseventsComponent,
    WhitespaceTrimDirective,
    FieldselectorComponent,
    HerocardComponent,
    SorticonComponent,
    BasecardComponent,
    LinegraphComponent,
    FlipComponent,
    ErrorTooltipDirective,
    TablevieweditHTMLComponent,
    RoleGuardDirective,
    FeatureGuardDirective,
    AdvancedSearchComponent,
    LabeleditorComponent,
    TimeRangeComponent,
    UIChartComponent,
    OrderedlistComponent,
    ChipsComponent,
    FormChipsComponent,
    AlertstableComponent,
    EventstableComponent,
    SyslogComponent,
    TelemetrychartComponent,
    TelemetrycharteditComponent,
    TelemetrychartviewComponent,
    AlertIndicationBarComponent,
    DscprofilesetterComponent,
    ExportLogsComponent
  ],
  providers: [
    DomHandler
  ]
})
export class SharedModule { }
