import {  ComponentFixture, TestBed } from '@angular/core/testing';
import { configureTestSuite } from 'ng-bullet';

import { MetricpolicyComponent } from './metricpolicy.component';

describe('MetricpolicyComponent', () => {
  let component: MetricpolicyComponent;
  let fixture: ComponentFixture<MetricpolicyComponent>;

  configureTestSuite(() => {
     TestBed.configureTestingModule({
      declarations: [MetricpolicyComponent]
    });
      });

  beforeEach(() => {
    fixture = TestBed.createComponent(MetricpolicyComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
