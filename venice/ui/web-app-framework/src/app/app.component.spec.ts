import { TestBed, async } from '@angular/core/testing';
import { RouterTestingModule } from '@angular/router/testing';
import { NgModule } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { HttpClient } from '@angular/common/http';
import { HttpClientTestingModule } from '@angular/common/http/testing';
import { Router } from '@angular/router';

import { MaterialModule } from './material.module';

import { AppComponent } from './app.component';
import { TestbarchartComponent } from '@testapp/testbarchart/testbarchart.component';
import { BarchartComponent} from '@modules/widgets/d3charts/barchart/barchart.component';
import { TestplotlyComponent } from '@testapp/testplotly/testplotly.component';
import { PlotlyComponent} from '@modules/widgets/plotlychart/plotly/plotly.component';
import { PlotlyimageComponent} from '@modules/widgets/plotlychart/plotlyimage/plotlyimage.component';

import { RecursivelistComponent} from '@modules/widgets/recursivelist/recursivelist/recursivelist.component';
import { TestRecursiveListComponent } from '@testapp/test-recursive-list/test-recursive-list.component';

describe('AppComponent', () => {
  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [
        AppComponent,
        BarchartComponent,
        TestbarchartComponent,
        TestplotlyComponent,
        PlotlyComponent,
        PlotlyimageComponent,
        RecursivelistComponent,
        TestRecursiveListComponent
      ],
      imports: [
        // Other modules...
        RouterTestingModule,
        FormsModule,
        HttpClientTestingModule,

        MaterialModule
    ],
providers: [
],
    }).compileComponents();
  }));
  it('should create the app', async(() => {
    const fixture = TestBed.createComponent(AppComponent);
    const app = fixture.debugElement.componentInstance;
    expect(app).toBeTruthy();
  }));
 /*  it(`should have as title 'app'`, async(() => {
    const fixture = TestBed.createComponent(AppComponent);
    const app = fixture.debugElement.componentInstance;
    expect(app.title).toEqual('app');
  }));
  it('should render title in a h1 tag', async(() => {
    const fixture = TestBed.createComponent(AppComponent);
    fixture.detectChanges();
    const compiled = fixture.debugElement.nativeElement;
    expect(compiled.querySelector('h1').textContent).toContain('Welcome to app!');
  })); */
});
