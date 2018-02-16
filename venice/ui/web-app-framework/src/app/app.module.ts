import { BrowserModule } from '@angular/platform-browser';
import { BrowserAnimationsModule } from '@angular/platform-browser/animations';
import { NgModule } from '@angular/core';

import { MaterialModule } from './material.module';

import { routing } from './app.routing';
import { AppComponent } from './app.component';

// import our module
import { WidgetsModule } from './modules/widgets/widgets.module';
import { TestbarchartComponent } from './test/testbarchart/testbarchart.component';
import { TestplotlyComponent } from './test/testplotly/testplotly.component';
import { TestRecursiveListComponent } from './test/test-recursive-list/test-recursive-list.component';
@NgModule({
  declarations: [
    AppComponent,
    TestbarchartComponent,
    TestplotlyComponent,
    TestRecursiveListComponent
  ],
  imports: [
    routing,
    BrowserModule,
    WidgetsModule ,

    MaterialModule,
    BrowserAnimationsModule
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
