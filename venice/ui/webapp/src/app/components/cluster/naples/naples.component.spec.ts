import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { NaplesComponent } from './naples.component';

describe('NaplesComponent', () => {
  let component: NaplesComponent;
  let fixture: ComponentFixture<NaplesComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [NaplesComponent]
    })
      .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(NaplesComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
