import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { MetricpolicyComponent } from './metricpolicy.component';

describe('MetricpolicyComponent', () => {
  let component: MetricpolicyComponent;
  let fixture: ComponentFixture<MetricpolicyComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [MetricpolicyComponent]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(MetricpolicyComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
