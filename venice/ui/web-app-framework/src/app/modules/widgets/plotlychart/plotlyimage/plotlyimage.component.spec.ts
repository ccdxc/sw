import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { PlotlyimageComponent } from './plotlyimage.component';
import { PlotlyComponent } from '../plotly/plotly.component';
describe('PlotlyimageComponent', () => {
  let component: PlotlyimageComponent;
  let fixture: ComponentFixture<PlotlyimageComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ PlotlyimageComponent, PlotlyComponent ]

    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(PlotlyimageComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
