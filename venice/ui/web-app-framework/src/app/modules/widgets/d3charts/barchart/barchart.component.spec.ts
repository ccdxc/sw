
import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { BarchartComponent } from './barchart.component';

describe('BarchartComponent', () => {
  let component: BarchartComponent;
  let fixture: ComponentFixture<BarchartComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [  BarchartComponent],
     // imports:[WidgetsModule]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(BarchartComponent);
    component = fixture.componentInstance;
    // fixture.detectChanges(); //DON't use this line. It will cause problem in UT
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
