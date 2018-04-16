import { BrowserModule } from '@angular/platform-browser';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { NgModule } from '@angular/core';
import { FormsModule } from '@angular/forms';

import { MaterialModule } from './material.module';
import { PrimengModule } from './primeng.module';

import { routing } from './app.routing';
import { AppComponent } from './app.component';

// import our module
import { WidgetsModule } from './modules/widgets/widgets.module';
import { TestbarchartComponent } from './test/testbarchart/testbarchart.component';
import { TestplotlyComponent } from './test/testplotly/testplotly.component';
import { TestRecursiveListComponent } from './test/test-recursive-list/test-recursive-list.component';
import { FlexLayoutModule } from '@angular/flex-layout';
import { TestSearchboxComponent } from './test/test-searchbox/test-searchbox.component';

@NgModule({
  declarations: [
    AppComponent,
    TestbarchartComponent,
    TestplotlyComponent,
    TestRecursiveListComponent,
    TestSearchboxComponent
  ],
  imports: [
    FormsModule,
    routing,
    BrowserModule,
    WidgetsModule ,

    MaterialModule,
    PrimengModule,
    BrowserAnimationsModule,
    FlexLayoutModule
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
