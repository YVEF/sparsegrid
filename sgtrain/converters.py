import models
import os
import torch
import pandas as pd
import gzip
import io
import sgfiles


def sg_convert_weights():
    model = models.SgModel()
    if not os.path.isfile(sgfiles.SG_MODEL_WEIGHTS_FILE):
        return 1

    if os.path.isfile(sgfiles.WEIGHT_FILE):
        os.remove(sgfiles.WEIGHT_FILE)

    model.load_state_dict(torch.load(sgfiles.SG_MODEL_WEIGHTS_FILE, weights_only=True))
    state_ = model.state_dict()
    data = []
    for t in state_:
        param_ = state_[t].numpy().flatten()
        shape_ = [*state_[t].shape, 1] if len(state_[t].shape) == 1 else [*state_[t].shape]
        data.append([t, *shape_, *param_])

    df = pd.DataFrame(data)
    # df.to_csv("hi.csv", index=False, header=False)
    bb = bytes(df.to_csv(index=False, header=False), encoding='utf-8')
    res = gzip.compress(bb)
    with open(sgfiles.WEIGHT_FILE, "wb") as file:
        file.write(res)

    return 0
